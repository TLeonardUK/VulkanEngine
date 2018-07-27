using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Diagnostics;

namespace ShaderBuilder
{
    class Program
    {
        public static void Main(string[] args)
        {
            System.Console.ForegroundColor = ConsoleColor.DarkGray;

            string shaderBuilderPath = Path.GetFullPath("../ThirdParty/vulkan/1.1.77.0/Bin32/glslangValidator.exe");
            string outputDirectory = Path.GetFullPath("../Temp/Assets/");
            string inputDirectory = Path.GetFullPath("../Assets/");

            bool bSuccess = true;

            var fragFiles = Directory.EnumerateFiles(inputDirectory, "*.frag", SearchOption.AllDirectories);
            foreach (string file in fragFiles)
            {
                string outputFile = outputDirectory + file.Substring(inputDirectory.Length) + ".spirv";
                if (!CompileShader(file, outputFile, shaderBuilderPath))
                {
                    bSuccess = false;
                    break;
                }
            }

            if (bSuccess)
            {
                var vertFiles = Directory.EnumerateFiles(inputDirectory, "*.vert", SearchOption.AllDirectories);
                foreach (string file in vertFiles)
                {
                    string outputFile = outputDirectory + file.Substring(inputDirectory.Length) + ".spirv";
                    if (!CompileShader(file, outputFile, shaderBuilderPath))
                    {
                        bSuccess = false;
                        break;
                    }
                }
            }

            if (bSuccess)
            {
                System.Console.ForegroundColor = ConsoleColor.Green;
                System.Console.WriteLine("Finished. Press any key to close.");
            }
            else
            {
                System.Console.ForegroundColor = ConsoleColor.Red;
                System.Console.WriteLine("Compilation failed. Press any key to close.");
            }        
            System.Console.ReadKey();
        }

        public static bool CompileShader(string input, string output, string compiler)
        {
            System.Console.ForegroundColor = ConsoleColor.White;
            System.Console.WriteLine("Compiling: " + input);
            System.Console.ForegroundColor = ConsoleColor.DarkGray;

            Directory.CreateDirectory(Path.GetDirectoryName(output));

            Process process = new Process();
            process.StartInfo.CreateNoWindow = true;
            process.StartInfo.FileName = compiler;
            process.StartInfo.Arguments = "-V " + input + " -o " + output;
            process.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.RedirectStandardError = true;
            process.StartInfo.UseShellExecute = false;
            process.Start();
            System.Console.Write(process.StandardOutput.ReadToEnd());
            System.Console.Write(process.StandardError.ReadToEnd());
            System.Console.WriteLine("");
            process.WaitForExit();

            return (process.ExitCode == 0);
        }
    }
}
