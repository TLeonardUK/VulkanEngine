namespace ModelImporter
{
    partial class ImporterForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.label1 = new System.Windows.Forms.Label();
            this.sourcePathBox = new System.Windows.Forms.TextBox();
            this.browseButton = new System.Windows.Forms.Button();
            this.importButton = new System.Windows.Forms.Button();
            this.statusBox = new System.Windows.Forms.TextBox();
            this.assetRootPathBox = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.browseAssetRootButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(9, 13);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(60, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Source File";
            // 
            // sourcePathBox
            // 
            this.sourcePathBox.Location = new System.Drawing.Point(88, 10);
            this.sourcePathBox.Name = "sourcePathBox";
            this.sourcePathBox.ReadOnly = true;
            this.sourcePathBox.Size = new System.Drawing.Size(287, 20);
            this.sourcePathBox.TabIndex = 1;
            this.sourcePathBox.Text = "D:\\Git\\VulkanEngine\\Assets\\Engine\\Models\\Bistro\\exterior.obj";
            // 
            // browseButton
            // 
            this.browseButton.Location = new System.Drawing.Point(381, 8);
            this.browseButton.Name = "browseButton";
            this.browseButton.Size = new System.Drawing.Size(41, 23);
            this.browseButton.TabIndex = 2;
            this.browseButton.Text = "...";
            this.browseButton.UseVisualStyleBackColor = true;
            this.browseButton.Click += new System.EventHandler(this.browseButton_Click);
            // 
            // importButton
            // 
            this.importButton.Location = new System.Drawing.Point(347, 248);
            this.importButton.Name = "importButton";
            this.importButton.Size = new System.Drawing.Size(75, 23);
            this.importButton.TabIndex = 3;
            this.importButton.Text = "Import";
            this.importButton.UseVisualStyleBackColor = true;
            this.importButton.Click += new System.EventHandler(this.ImportClicked);
            // 
            // statusBox
            // 
            this.statusBox.Location = new System.Drawing.Point(88, 62);
            this.statusBox.Multiline = true;
            this.statusBox.Name = "statusBox";
            this.statusBox.ReadOnly = true;
            this.statusBox.Size = new System.Drawing.Size(334, 180);
            this.statusBox.TabIndex = 5;
            // 
            // assetRootPathBox
            // 
            this.assetRootPathBox.Location = new System.Drawing.Point(88, 36);
            this.assetRootPathBox.Name = "assetRootPathBox";
            this.assetRootPathBox.ReadOnly = true;
            this.assetRootPathBox.Size = new System.Drawing.Size(287, 20);
            this.assetRootPathBox.TabIndex = 7;
            this.assetRootPathBox.Text = "D:\\Git\\VulkanEngine\\Assets";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(9, 39);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(59, 13);
            this.label2.TabIndex = 6;
            this.label2.Text = "Asset Root";
            // 
            // browseAssetRootButton
            // 
            this.browseAssetRootButton.Location = new System.Drawing.Point(381, 34);
            this.browseAssetRootButton.Name = "browseAssetRootButton";
            this.browseAssetRootButton.Size = new System.Drawing.Size(41, 23);
            this.browseAssetRootButton.TabIndex = 8;
            this.browseAssetRootButton.Text = "...";
            this.browseAssetRootButton.UseVisualStyleBackColor = true;
            this.browseAssetRootButton.Click += new System.EventHandler(this.browseAssetRootButton_Click);
            // 
            // ImporterForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(434, 289);
            this.Controls.Add(this.browseAssetRootButton);
            this.Controls.Add(this.assetRootPathBox);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.statusBox);
            this.Controls.Add(this.importButton);
            this.Controls.Add(this.browseButton);
            this.Controls.Add(this.sourcePathBox);
            this.Controls.Add(this.label1);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "ImporterForm";
            this.ShowIcon = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Model Importer";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox sourcePathBox;
        private System.Windows.Forms.Button browseButton;
        private System.Windows.Forms.Button importButton;
        private System.Windows.Forms.TextBox statusBox;
        private System.Windows.Forms.TextBox assetRootPathBox;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button browseAssetRootButton;
    }
}

