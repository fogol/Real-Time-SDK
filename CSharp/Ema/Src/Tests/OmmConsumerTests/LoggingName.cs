using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Xunit.Sdk;

namespace LSEG.Ema.Access.Tests.OmmConsumerTests
{

    public class LogTestNameAttribute : BeforeAfterTestAttribute
    {
        public override void Before(MethodInfo methodUnderTest)
        {
            Console.WriteLine($">>>>>>>>>>>>>>>>>>>>>>>>>>> Starting test: {methodUnderTest.Name}");
        }

        public override void After(MethodInfo methodUnderTest)
        {
            Console.WriteLine($">>>>>>>>>>>>>>>>>>>>>>>>> Finished test: {methodUnderTest.Name}");
        }
    }

}
