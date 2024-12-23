package polyraygameengine.misc;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;

public class NativeFileChooser {

    public static final int APPROVE_OPTION = 0;

    private File selectedFile;
    private NativeFileFilter filter = NativeFileFilter.ALL_FILES;
    private String title = "Open File";

    public NativeFileChooser() {
        if (System.getProperty("os.name").toLowerCase().contains("win")) {
            throw new RuntimeException("Native File Chooser currently only supports Windows");
        }
    }

    public void setFileFilter(NativeFileFilter filter) {
        this.filter = filter;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public int showOpenDialog() {
        try {
            String command;
            if (filter == NativeFileFilter.DIRECTORIES) {
                command = String.format(
                        "powershell -Command \"Add-Type -AssemblyName System.Windows.Forms; "
                        + "$openFileDialog = New-Object System.Windows.Forms.OpenFileDialog; "
                        + "$openFileDialog.Filter = '%s';"
                        + "$openFileDialog.Title = '%s';"
                        + "$openFileDialog.CheckFileExists = 0;"
                        + "$openFileDialog.ValidateNames = 0;"
                        + "$openFileDialog.ShowDialog() | Out-Null; $openFileDialog.FileName\"",
                        filter.filter, this.title
                );
            } else {
                command = String.format(
                        "powershell -Command \"Add-Type -AssemblyName System.Windows.Forms; "
                        + "$openFileDialog = New-Object System.Windows.Forms.OpenFileDialog; "
                        + "$openFileDialog.Filter = '%s';"
                        + "$openFileDialog.Title = '%s';"
                        + "$openFileDialog.ShowDialog() | Out-Null; $openFileDialog.FileName\"",
                        filter.filter, this.title
                );
            }

            Process process = Runtime.getRuntime().exec(command);

            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String filePath = reader.readLine();
            if (!filePath.isEmpty()) {
                this.selectedFile = new File(filePath);
                return APPROVE_OPTION;
            }
            return 1;
        } catch (IOException e) {
        }
        return 1;
    }

    public int showSaveDialog() {
        try {
            String command = String.format("powershell -Command \"Add-Type -AssemblyName System.Windows.Forms; "
                    + "$saveFileDialog = New-Object System.Windows.Forms.SaveFileDialog; "
                    + "$saveFileDialog.Filter = '%s';"
                    + "$openFileDialog.Title = '%s';"
                    + "$saveFileDialog.ShowDialog() | Out-Null; $saveFileDialog.FileName\"",
                    filter.filter, this.title
            );

            Process process = Runtime.getRuntime().exec(command);

            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String filePath = reader.readLine();
            if (filePath != null && !filePath.isEmpty()) {
                this.selectedFile = new File(filePath);
                return APPROVE_OPTION;
            }
            return 1;
        } catch (IOException e) {
        }
        return 1;
    }

    public File getSelectedFile() {
        return this.selectedFile;
    }
}
