using System;
using Xunit;
using Xunit.Sdk;

namespace LSEG.Eta.Tests.Xunit
{
    public class CollectionAssertUtilTests
    {
        public class AssertCollectionUnorderedTests
        {
            [Fact]
            public void Should_pass_for_the_same_order_of_elements()
            {
                var elements = new[] { 2, 5 };
                AssertCollectionUnordered(elements, x => Assert.Equal(2, x), x => Assert.Equal(5, x));
            }

            [Fact]
            public void Should_pass_for_the_same_single_element()
            {
                var elements = new[] { 5 };
                AssertCollectionUnordered(elements, x => Assert.Equal(5, x));
            }

            [Fact]
            public void Should_pass_for_the_different_order_of_elements()
            {
                var elements = new[] { 5, 2 };
                AssertCollectionUnordered(elements, x => Assert.Equal(2, x), x => Assert.Equal(5, x));
            }

            [Fact]
            public void Should_throw_exception_when_no_element_inspections_passed_to_method()
            {
                var elements = new[] { 5, 2 };
                var e = Assert.Throws<ArgumentException>(() => AssertCollectionUnordered(elements));
                Assert.StartsWith("No element inspections passed to the method", e.Message);
            }

            [Theory]
            [InlineData(new int[] { })]
            [InlineData(new int[] { 5 })]
            [InlineData(new int[] { 1 })]
            [InlineData(new int[] { 5, 2, 3 })]
            public void Should_fail_for_the_different_number_of_elements(int[] elements)
            {
                var e = Assert.Throws<FailException>(() =>
                    AssertCollectionUnordered(elements, x => Assert.Equal(2, x), x => Assert.Equal(5, x)));
                Assert.StartsWith("Collection & its element inspections have different lengths", e.Message);
            }

            [Theory]
            [InlineData(new int[] { 5, 3 })]
            [InlineData(new int[] { 3, 2 })]
            [InlineData(new int[] { 9, 1 })]
            public void Should_fail_for_the_different_set_of_elements(int[] elements)
            {
                var e = Assert.Throws<FailException>(() =>
                    AssertCollectionUnordered(elements, x => Assert.Equal(2, x), x => Assert.Equal(5, x)));
                Assert.StartsWith("Collection has unsatisfied inspections", e.Message);
            }
        }
    }
}
