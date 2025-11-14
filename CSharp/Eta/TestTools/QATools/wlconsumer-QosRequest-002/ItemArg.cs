/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Rdm;

namespace LSEG.Eta.Example.VACommon;

/// <summary>
/// Item argument class for the Value Add consumer and non-interactive provider applications.
/// </summary>
public class ItemArg
{
    public DomainType Domain;
    public string? ItemName;
    public bool EnablePrivateStream;
    public bool EnableView;
    public bool EnableSnapshot;
    public bool EnableMsgKeyInUpdates;
    public int ViewId;
    public bool SymbolListData;

    public ItemArg(DomainType domain, string itemName, bool enablePrivateStream)
    {
        Domain = domain;
        ItemName = itemName;
        EnablePrivateStream = enablePrivateStream;
    }

    // APIQA
    public ItemArg(DomainType domain, string itemName, bool enablePrivateStream, bool enableView, bool enableSnapshot, int viewId, bool enableMsgKeyInUpdates)
    {
        Domain = domain;
        ItemName = itemName;
        EnablePrivateStream = enablePrivateStream;
        EnableView = enableView;
        EnableSnapshot = enableSnapshot;
        EnableMsgKeyInUpdates = enableMsgKeyInUpdates;
        ViewId = viewId;
    }

    // END APIQA

    public ItemArg()
    {
    }
}
