using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Threading.Tasks;
using System.Diagnostics;

namespace ShaderBuilder
{
    /// <summary>
    /// 
    /// </summary>
    public interface IShaderCompiler
    {
        /// <summary>
        /// 
        /// </summary>
        string RootPath
        {
            get;
            set;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        Shader Compile(string inputPath);
    }
}
