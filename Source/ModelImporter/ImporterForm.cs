using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Windows.Forms;

namespace ModelImporter
{
    public partial class ImporterForm : Form
    {
        public ImporterForm()
        {
            InitializeComponent();
        }

        private void AddStatus(string text)
        {
            statusBox.AppendText(text + "\n");
        }

        private void ClearStatus()
        {
            statusBox.Text = "";
        }

        private string GetAssetPath(string path)
        {
            string assetRoot = assetRootPathBox.Text;
            if (path.StartsWith(assetRoot))
            {
                path = path.Substring(assetRoot.Length);
            }
            return path.Replace('\\', '/').TrimStart('/');
        }

        private string GetRootFolder()
        {
            return Path.GetDirectoryName(sourcePathBox.Text);
        }

        private Dictionary<string, string> ParseMaterialFile(string path)
        {
            AddStatus("Parsing material file: " + path);

            string[] lines = File.ReadAllLines(path);
            string currentMaterialName = "";

            Dictionary<string, string> materialMap = new Dictionary<string, string>();

            foreach (string unsantizedLine in lines)
            {
                string line = unsantizedLine.Trim();

                if (line.StartsWith("newmtl"))
                {
                    currentMaterialName = line.Substring(6).Trim();
                }
                else if (line.StartsWith("map_Kd"))
                {
                    string texturePath = line.Substring(6).Trim();
                    if (!Path.IsPathRooted(texturePath))
                    {
                        texturePath = GetRootFolder() + "/" + texturePath;
                    }

                    string materialAssetPath = GetRootFolder() + "/Materials/" + currentMaterialName + ".json";
                    string textureAssetPath = GetRootFolder() + "/Textures/" + Path.GetFileNameWithoutExtension(texturePath) + ".json";

                    WriteTextureFile(textureAssetPath, texturePath);
                    WriteMaterialFile(materialAssetPath, textureAssetPath);

                    if (!materialMap.ContainsKey(currentMaterialName))
                    {
                        materialMap.Add(currentMaterialName, materialAssetPath);
                    }
                }
            }

            return materialMap;
        }

        private void WriteTextureFile(string path, string texturePath)
        {
            AddStatus("Writing texture file: " + path);

            List<string> lines = new List<string>();
            lines.Add("{");
            lines.Add("\t\"Type\": \"Texture\",");
            lines.Add("\t\"ImagePath\": \"" + GetAssetPath(texturePath) + "\",");
            lines.Add("\t\"AddressModeU\": \"Repeat\",");
            lines.Add("\t\"AddressModeV\": \"Repeat\",");
            lines.Add("\t\"AddressModeW\": \"Repeat\"");
            lines.Add("}");

            File.WriteAllLines(path, lines);
        }

        private void WriteMaterialFile(string path, string texturePath)
        {
            AddStatus("Writing material file: " + path);

            List<string> lines = new List<string>();
            lines.Add("{");
            lines.Add("\t\"Type\": \"Material\",");
            lines.Add("\t\"ShaderPath\": \"Engine/Shaders/default.json\",");
            lines.Add("\t\"Bindings\":");
            lines.Add("\t{");
            lines.Add("\t\t\"AlbedoTexture\":");
            lines.Add("\t\t{");
            lines.Add("\t\t\t\"Format\": \"Texture\",");
            lines.Add("\t\t\t\"Value\": \"" + GetAssetPath(texturePath) + "\"");
            lines.Add("\t\t}");
            lines.Add("\t}");
            lines.Add("}");

            File.WriteAllLines(path, lines);
        }

        private void WriteModelFile(string path, string modelPath, Dictionary<string, string> materialMap)
        {
            AddStatus("Writing model file: " + path);

            List<string> lines = new List<string>();
            lines.Add("{");
            lines.Add("\t\"Type\": \"Model\",");
            lines.Add("\t\"ModelPath\": \"" + GetAssetPath(modelPath) + "\",");
            lines.Add("\t\"MaterialMapping\":");
            lines.Add("\t{");
            for (int i = 0; i < materialMap.Keys.Count; i++)
            {
                string key = materialMap.Keys.ElementAt(i);
                string value = materialMap.Values.ElementAt(i);
                lines.Add("\t\t\"" + key + "\": \"" + GetAssetPath(value) + "\"" + (i == materialMap.Keys.Count - 1 ? "" : ","));
            }
            lines.Add("\t}");
            lines.Add("}");
            
            File.WriteAllLines(path, lines);
        }

        private void ImportClicked(object sender, EventArgs e)
        {
            string materialFile = Path.ChangeExtension(sourcePathBox.Text, ".mtl");

            Directory.CreateDirectory(GetRootFolder() + "/Materials/");
            Directory.CreateDirectory(GetRootFolder() + "/Textures/");

            Dictionary<string, string> materialMap = new Dictionary<string, string>();
            if (File.Exists(materialFile))
            {
                materialMap = ParseMaterialFile(materialFile);
            }

            string outputFile = Path.ChangeExtension(sourcePathBox.Text, ".json");
            WriteModelFile(outputFile, sourcePathBox.Text, materialMap);
        }

        private void browseAssetRootButton_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog dialog = new FolderBrowserDialog();
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                assetRootPathBox.Text = dialog.SelectedPath;
            }
        }

        private void browseButton_Click(object sender, EventArgs e)
        {
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.Title = "Select file ...";
            dialog.Filter = "Obj Model|*.obj";
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                sourcePathBox.Text = dialog.FileName;
            }
        }
    }
}
