using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ShaderBuilder
{
    /// <summary>
    /// 
    /// </summary>
    public class ShaderPreprocessor
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
        public string[] Postfix
        {
            get;
            set;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="shader"></param>
        /// <param name="file"></param>
        /// <returns></returns>
        public bool ProcessFile(Shader shader, string file, out string output)
        {
            output = "";

            List<string> lines = File.ReadAllLines(file).ToList();

            int dependentFileIndex = shader.GetDependentFileIndex(file);

            StringBuilder builder = new StringBuilder();
            builder.AppendLine("#line 1 " + dependentFileIndex);

            for (int i = 0; i < lines.Count; i++)
            {
                string trimmedLine = lines[i];

                if (trimmedLine.StartsWith("#include "))
                {
                    int startQuote = trimmedLine.IndexOf('\"');
                    if (startQuote >= 0)
                    {
                        int endQuote = trimmedLine.IndexOf('\"', startQuote + 1);
                        if (endQuote >= 0)
                        {
                            string path = trimmedLine.Substring(startQuote + 1, (endQuote - startQuote) - 1);
                            System.Console.WriteLine("Including: " + path);

                            if (!File.Exists(path))
                            {
                                path = Path.Combine(Path.GetDirectoryName(file), path);
                                if (!File.Exists(path))
                                {
                                    path = Path.Combine(RootPath, path);
                                    if (!File.Exists(path))
                                    {
                                        shader.Output += "Unable to find included path '" + path + "'.";
                                        return false;
                                    }
                                }
                            }

                            string includedLines = "";
                            if (!ProcessFile(shader, path, out includedLines))
                            {
                                return false;
                            }

                            builder.Append(includedLines);
                            builder.AppendLine("#line " + (i + 1 + 1) + " " + dependentFileIndex);

                            continue;
                        }
                        else
                        {
                            shader.Output += "#include macro has invalid syntax.";
                            return false;
                        }
                    }
                    else
                    {
                        shader.Output += "#include macro has invalid syntax.";
                        return false;
                    }
                }

                builder.AppendLine(lines[i]);
            }

            output = builder.ToString();
            return true;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public bool Process(Shader shader)
        {
            List<string> lines = File.ReadAllLines(shader.InputFile).ToList();

            StringBuilder builder = new StringBuilder();

            foreach (string line in Postfix)
            {
                builder.AppendLine(line);
            }

            string includedLines = "";
            if (!ProcessFile(shader, shader.InputFile, out includedLines))
            {
                return false;
            }

            builder.Append(includedLines);

            string result = builder.ToString();

            File.WriteAllText(shader.PostProcessedFile, result);

            return true;
        }
    }
}
