/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Linq;
using Xunit.Abstractions;
using Xunit.Sdk;

namespace LSEG.Eta.Tests.Xunit;

/// <summary>
/// Allows to execute tests in particular order according to <see cref="Priority"/> property value.
/// Could be applied either to test method, test class or both. If both test class &amp; test methods have this
/// attribute, Priority value is taken from method.
/// </summary>
[Obsolete("Don't commit usage of this attribute to VCS. It's intended for debugging only.")]
[AttributeUsage(AttributeTargets.Method | AttributeTargets.Class, AllowMultiple = false)]
public class TestPriorityAttribute : Attribute
{
    public uint Priority { get; private set; }

    public TestPriorityAttribute(uint priority) => Priority = priority;
}

/// <summary>
/// In order to make <see cref="TestPriorityAttribute"/> work you should "apply"
/// this entity to particular test class via <see cref="global::Xunit.TestCaseOrdererAttribute"/> as following:
/// <code>
/// [TestCaseOrderer(
///     ordererTypeName: "LSEG.Eta.Tests.Xunit.PriorityOrderer",
///     ordererAssemblyName: "ESDK.Tests")]
/// public class MyTests
/// { ...
/// </code>
/// Or to whole test assembly:
/// <code>
/// [assembly: TestCaseOrderer(
///     ordererTypeName: "LSEG.Eta.Tests.Xunit.PriorityOrderer",
///     ordererAssemblyName: "ESDK.Tests")]
/// </code>
/// </summary>
[Obsolete("Don't commit usage of this attribute to VCS. It's intended for debugging only.")]
public class PriorityOrderer : ITestCaseOrderer
{
    public IEnumerable<TTestCase> OrderTestCases<TTestCase>(
        IEnumerable<TTestCase> testCases) where TTestCase : ITestCase
    {
        string assemblyQualifiedName = typeof(TestPriorityAttribute).AssemblyQualifiedName;
        var sortedMethods = new SortedDictionary<uint, List<TTestCase>>();
        foreach (TTestCase testCase in testCases)
        {
            var priority = 
                testCase.TestMethod.Method
                    .GetCustomAttributes(assemblyQualifiedName)
                    .FirstOrDefault()
                    ?.GetNamedArgument<uint>(nameof(TestPriorityAttribute.Priority))
                ?? testCase.TestMethod.TestClass.Class
                    .GetCustomAttributes(assemblyQualifiedName)
                    .FirstOrDefault()
                    ?.GetNamedArgument<uint>(nameof(TestPriorityAttribute.Priority))
                ?? uint.MaxValue;

            GetOrCreate(sortedMethods, priority).Add(testCase);
        }

        return sortedMethods.Keys.SelectMany(
                priority => sortedMethods[priority]
                    .OrderBy(testCase => testCase.TestMethod.Method.Name)
                    .ThenBy(testCase => testCase.DisplayName));
    }

    private static TValue GetOrCreate<TKey, TValue>(
        IDictionary<TKey, TValue> dictionary, TKey key)
        where TKey : struct
        where TValue : new() =>
        dictionary.TryGetValue(key, out TValue result)
            ? result
            : (dictionary[key] = new TValue());
}
