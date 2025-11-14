/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using Xunit;

namespace LSEG.Eta.ValuedAdd.Tests;

public class Provider : TestReactorComponent, IProviderCallback
{
    public Provider(TestReactor testReactor, bool disposeReactor = false) : base(testReactor, disposeReactor)
    {
        ReactorRole = new ProviderRole();
    }

    public Provider() : this(new TestReactor(), true)
    { }

    public ProviderRole Role => (ProviderRole)ReactorRole;

    public Provider WithDefaultRole()
    {
        Role.ChannelEventCallback = this;
        Role.LoginMsgCallback = this;
        Role.DirectoryMsgCallback = this;
        Role.DictionaryMsgCallback = this;
        Role.DefaultMsgCallback = this;

        return this;
    }

    public virtual ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
    {
        return TestReactor.HandleChannelEvent(evt);
    }

    public virtual ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent evt)
    {
        return TestReactor.HandleDefaultMsgEvent(evt);
    }

    public virtual ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent evt)
    {
        return TestReactor.HandleLoginMsgEvent(evt);
    }

    public virtual ReactorCallbackReturnCode RdmDirectoryMsgCallback(RDMDirectoryMsgEvent evt)
    {
        return TestReactor.HandleDirectoryMsgEvent(evt);
    }

    public virtual ReactorCallbackReturnCode RdmDictionaryMsgCallback(RDMDictionaryMsgEvent evt)
    {
        return TestReactor.HandleDictionaryMsgEvent(evt);
    }

    public void Accept(ConsumerProviderSessionOptions opts, TimeSpan timeout) =>
        TestReactor.Accept(opts, this, timeout);

    public void Accept(ConsumerProviderSessionOptions opts) =>
        TestReactor.Accept(opts, this);

    public Provider RespondToLoginRequest(Action<LoginRefresh> customizeResponse = null, ReactorSubmitOptions submitOptions = null)
    {
        var loginMsgEvent = TestReactor.PollReactorEvent<RDMLoginMsgEvent>();
        Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);

        // send default login refresh
        var loginRequest = loginMsgEvent.LoginMsg.LoginRequest;
        var loginRefresh = new LoginRefresh()
        {
            Solicited = true,
            UserName = loginRequest.UserName,
            StreamId = loginRequest.StreamId,
            HasAttrib = true,
            HasFeatures = true,
            SupportedFeatures = {
                HasSupportOptimizedPauseResume = true,
                SupportOptimizedPauseResume = 1,
                HasSupportViewRequests = true,
                SupportViewRequests = 1,
                HasSupportPost = true,
                SupportOMMPost = 1,
                // is this relevant for C# implementation?
                HasSupportStandby = true,
                SupportStandby = 1,
                HasSupportStandbyMode = true,
                SupportStandbyMode = 3,
            },
        };
        SetStateText(loginRefresh.State, "Login OK");
        customizeResponse?.Invoke(loginRefresh);

        SubmitAndDispatch(loginRefresh, submitOptions ?? new ReactorSubmitOptions());

        // Save the stream ID used by each component to open the login stream (may be
        // different if the watchlist is enabled).
        DefaultSessionLoginStreamId = loginRequest.StreamId;

        return this;
    }

    public Provider RespondToDirectoryRequest(Action<DirectoryRefresh> customizeResponse = null, ReactorSubmitOptions submitOptions = null)
    {
        var directoryRequestMsg = TestReactor.PollReactorEvent<RDMDirectoryMsgEvent>().DirectoryMsg;
        Assert.Equal(DirectoryMsgType.REQUEST, directoryRequestMsg.DirectoryMsgType);

        // send a default directory refresh.
        var directoryRequest = directoryRequestMsg.DirectoryRequest;
        var directoryRefresh = new DirectoryRefresh()
        {
            StreamId = directoryRequest.StreamId,
            Filter = directoryRequest.Filter,
            Solicited = true,
            ClearCache = true,
        };
        SetStateText(directoryRefresh.State, "Source Directory Refresh Complete");
        var service = new Service();
        DefaultService.Copy(service);
        directoryRefresh.ServiceList.Add(service);
        customizeResponse?.Invoke(directoryRefresh);

        SubmitAndDispatch(directoryRefresh, submitOptions ?? new ReactorSubmitOptions());

        // Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled).
        DefaultSessionDirectoryStreamId = directoryRequest.StreamId;

        return this;
    }

    private static void SetStateText(State state, string text)
    {
        state.StreamState(StreamStates.OPEN);
        state.DataState(DataStates.OK);
        state.Code(StateCodes.NONE);
        state.Text().Data(text);
    }

    /* A default service that can be used when setting up a connection. */
    public static Service DefaultService { get; private set; }

    /* A second default service that can be used when setting up a connection. */
    public static Service DefaultService2 { get; private set; }

    static Provider()
    {
        DefaultService = new();
        DefaultService.Clear();
        DefaultService.HasInfo = true;
        DefaultService.HasState = true;
        DefaultService.ServiceId = 1;

        DefaultService.Info.ServiceName.Data("DEFAULT_SERVICE");

        DefaultService.Info.CapabilitiesList = new List<long>()
        {
            (long)DomainType.DICTIONARY, (long)DomainType.MARKET_PRICE,
            (long)DomainType.MARKET_BY_ORDER, (long)DomainType.SYMBOL_LIST,
            (long)DomainType.SYSTEM
        };

        DefaultService.Info.HasQos = true;
        DefaultService.Info.QosList = new List<Qos>();

        Qos qos = new();
        qos.Clear();
        qos.Timeliness(QosTimeliness.DELAYED);
        qos.Rate(QosRates.JIT_CONFLATED);
        qos.IsDynamic = false;
        qos.TimeInfo(0);
        qos.RateInfo(0);
        DefaultService.Info.QosList.Add(qos);

        qos.Clear();
        qos.Timeliness(QosTimeliness.REALTIME);
        qos.Rate(QosRates.TICK_BY_TICK);
        qos.IsDynamic = false;
        qos.TimeInfo(0);
        qos.RateInfo(0);
        DefaultService.Info.QosList.Add(qos);

        DefaultService.State.HasAcceptingRequests = true;
        DefaultService.State.AcceptingRequests = 1;
        DefaultService.State.ServiceStateVal = 1;

        DefaultService2 = new();
        DefaultService2.Clear();
        DefaultService2.HasInfo = true;
        DefaultService2.HasState = true;
        DefaultService2.ServiceId = 2;

        DefaultService2.Info.ServiceName.Data("DEFAULT_SERVICE2");

        DefaultService2.Info.CapabilitiesList = new List<long>()
        {
            (long)DomainType.DICTIONARY, (long)DomainType.MARKET_PRICE,
            (long)DomainType.MARKET_BY_ORDER, (long)DomainType.SYMBOL_LIST,
            (long)DomainType.SYSTEM
        };

        DefaultService2.Info.HasQos = true;
        DefaultService2.Info.QosList = new List<Qos>();
        qos.Clear();
        qos.Timeliness(QosTimeliness.DELAYED);
        qos.Rate(QosRates.JIT_CONFLATED);
        qos.IsDynamic = false;
        qos.TimeInfo(0);
        qos.RateInfo(0);
        DefaultService2.Info.QosList.Add(qos);

        qos.Clear();
        qos.Timeliness(QosTimeliness.REALTIME);
        qos.Rate(QosRates.TICK_BY_TICK);
        qos.IsDynamic = false;
        qos.TimeInfo(0);
        qos.RateInfo(0);
        DefaultService2.Info.QosList.Add(qos);

        DefaultService2.State.HasAcceptingRequests = true;
        DefaultService2.State.AcceptingRequests = 1;
        DefaultService2.State.ServiceStateVal = 1;
    }
}
