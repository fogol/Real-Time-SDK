using System;

namespace LSEG.Ema.Access.Tests.Utils
{
    public static class ComplexTypeExtensions
    {
        public static T Tap<T>(this T self, Action<T> action)
            where T : ComplexType
        {
            action(self);
            return self;
        }
    }
}
