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
    public class VulkanShaderCompiler : IShaderCompiler
    {
        /// <summary>
        /// 
        /// </summary>
        public string RootPath
        {
            get;
            set;
        }

        /// <summary>
        /// 
        /// </summary>
        public string GLSLCompilerPath
        {
            get;
            set;
        }

        /// <summary>
        /// 
        /// </summary>
        public string[] Postfix
        {
            get;
            set;
        }

        /// <summary>
        /// 
        /// </summary>
        public VulkanShaderCompiler()
        {
            // Preamble attached to top of shaders before compiling.
            Postfix = new string[] { 
                "#version 450",
                "#extension GL_ARB_separate_shader_objects : enable"
            };
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public Shader Compile(string inputPath)
        {
            Shader result = new Shader();
            result.InputFile = inputPath;
            result.PostProcessedFile = Path.GetTempFileName() + ".postprocess" + Path.GetExtension(inputPath);
            result.OutputFile = Path.GetTempFileName() + ".spirv";

            ShaderPreprocessor preprocessor = new ShaderPreprocessor();
            preprocessor.Postfix = Postfix;
            preprocessor.RootPath = RootPath;

            if (!preprocessor.Process(result))
            {
                return result;
            }

            Process process = new Process();
            process.StartInfo.CreateNoWindow = true;
            process.StartInfo.FileName = GLSLCompilerPath;
            process.StartInfo.Arguments = "-V " + result.PostProcessedFile + " -o " + result.OutputFile;
            process.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.RedirectStandardError = true;
            process.StartInfo.UseShellExecute = false;
            process.Start();

            result.Output += process.StandardOutput.ReadToEnd();
            result.Output += process.StandardError.ReadToEnd();
            process.WaitForExit();

            // Replace file-indexes with actual file names in error message (because glsl is garbage 
            // and doesn't allow a #line num file macro).
            for (int i = 0; i < result.DependentFiles.Count; i++)
            {
                string file = result.DependentFiles[i];
                result.Output = result.Output.Replace("ERROR: " + i + ":", "ERROR: " + file + ":");
                result.Output = result.Output.Replace("WARNING: " + i + ":", "WARNING: " + file + ":");
                result.Output = result.Output.Replace("INFO: " + i + ":", "INFO: " + file + ":");
                result.Output = result.Output.Replace("MESSAGE: " + i + ":", "MESSAGE: " + file + ":");
            }

            result.CompileFailed = (process.ExitCode != 0);

            return result;
        }
    }
}
