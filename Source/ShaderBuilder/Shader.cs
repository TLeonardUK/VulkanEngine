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
    public class Shader
    {
        /// <summary>
        /// 
        /// </summary>
        public string Output
        {
            get;
            set;
        }

        /// <summary>
        /// 
        /// </summary>
        public bool CompileFailed
        {
            get;
            set;
        }

        /// <summary>
        /// 
        /// </summary>
        public string InputFile
        {
            get;
            set;
        }

        /// <summary>
        /// 
        /// </summary>
        public string OutputFile
        {
            get;
            set;
        }

        /// <summary>
        /// 
        /// </summary>
        public string PostProcessedFile
        {
            get;
            set;
        }

        /// <summary>
        /// 
        /// </summary>
        public List<string> DependentFiles
        {
            get;
            set;
        } = new List<string>();

        /// <summary>
        /// 
        /// </summary>
        /// <param name="path"></param>
        /// <returns></returns>
        public int GetDependentFileIndex(string path)
        {
            path = path.ToLower();

            int index = DependentFiles.IndexOf(path);
            if (index >= 0)
            {
                return index;
            }
            else
            {
                DependentFiles.Add(path);
                return DependentFiles.Count - 1;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="outputFile"></param>
        /// <returns></returns>
        public bool Save(string savePath)
        {
            Directory.CreateDirectory(Path.GetDirectoryName(savePath));
            File.Copy(OutputFile, savePath, true);

            return true;
        }
    }
}
