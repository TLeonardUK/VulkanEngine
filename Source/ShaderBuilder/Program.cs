using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.IO;
using System.Diagnostics;

namespace ShaderBuilder
{
    public class Program
    {
        private string[] m_shaderFileExtensions = new string[] { ".frag", ".vert" };
        private string[] m_shaderFileWatchedExtensions = new string[] { ".frag", ".vert", ".h" };
        
        private AutoResetEvent m_filesChangedEvent = new AutoResetEvent(true);

        public List<string> GatherShaders(string rootPath, string[] extensions)
        {
            return
                Directory.EnumerateFiles(rootPath, "*.*", SearchOption.AllDirectories)
                .Where(s => extensions.Any(ext => ext == Path.GetExtension(s)))
                .ToList();
        }

        public bool CompileAllShaders(IShaderCompiler compiler, string outputDirectory, string inputDirectory)
        {
            List<string> shaders = GatherShaders(compiler.RootPath, m_shaderFileExtensions);

            bool bSuccess = true;

            foreach (string shaderPath in shaders)
            {
                System.Console.ForegroundColor = ConsoleColor.White;
                System.Console.WriteLine("Compiling: " + shaderPath);
                System.Console.ForegroundColor = ConsoleColor.DarkGray;

                Shader shader = compiler.Compile(shaderPath);

                System.Console.ForegroundColor = shader.CompileFailed ? ConsoleColor.Red : ConsoleColor.DarkGray;
                System.Console.WriteLine(shader.Output);
                System.Console.ForegroundColor = ConsoleColor.DarkGray;

                if (shader.CompileFailed)
                {
                    bSuccess = false;
                }
                else
                {
                    string outputFile = outputDirectory + shaderPath.Substring(compiler.RootPath.Length) + ".spirv";
                    if (!shader.Save(outputFile))
                    {
                        System.Console.ForegroundColor = ConsoleColor.Red;
                        System.Console.WriteLine("Failed to save output to: " + outputFile);
                        System.Console.ForegroundColor = ConsoleColor.DarkGray;

                        bSuccess = false;
                    }
                }
            }

            return bSuccess;
        }

        public void MonitorAndCompileChanges()
        {
            System.Console.ForegroundColor = ConsoleColor.DarkGray;

            string outputDirectory = Path.GetFullPath("../Temp/Assets/");
            string inputDirectory = Path.GetFullPath("../Assets/");

            FileSystemWatcher watcher = new FileSystemWatcher();
            watcher.IncludeSubdirectories = true;
            watcher.Path = inputDirectory;
            watcher.NotifyFilter = NotifyFilters.LastWrite;
            watcher.Filter = "*.*";
            watcher.Changed += new FileSystemEventHandler((object sender, FileSystemEventArgs e) =>
            {
                if (m_shaderFileWatchedExtensions.Contains(Path.GetExtension(e.FullPath)))
                {
                    m_filesChangedEvent.Set();
                }
            });
            watcher.EnableRaisingEvents = true;

            IShaderCompiler compiler = MakeShaderCompiler();
            compiler.RootPath = inputDirectory;

            while (true)
            {
                m_filesChangedEvent.WaitOne();

                bool bSuccess = CompileAllShaders(compiler, outputDirectory, inputDirectory);
                if (bSuccess)
                {
                    System.Console.ForegroundColor = ConsoleColor.Green;
                    System.Console.WriteLine("Compile success.");
                }
                else
                {
                    System.Console.ForegroundColor = ConsoleColor.Red;
                    System.Console.WriteLine("Compilation failed.");
                }

                System.Console.WriteLine();

                m_filesChangedEvent.Reset();
            }
        }

        public IShaderCompiler MakeShaderCompiler()
        {
            // todo: switch if we ever make more backends than vulkan

            VulkanShaderCompiler compiler = new VulkanShaderCompiler();
            compiler.GLSLCompilerPath = Path.GetFullPath("../ThirdParty/vulkan/1.1.77.0/Bin32/glslangValidator.exe");

            return compiler;
        }

        public static void Main(string[] args)
        {
            Program program = new Program();
            program.MonitorAndCompileChanges();
        }
    }
}
