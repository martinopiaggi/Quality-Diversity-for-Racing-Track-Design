# Define the list of source files
$sourceFiles = @(
  ".\tracks\dirt\output\output-trk.ac",
  ".\tracks\dirt\output\output.ac",
  ".\tracks\dirt\output\output.xml"
)

# Define the destination directory
$destinationDir = "C:\Program Files (x86)\torcs\tracks\dirt\output"

# Move each file to the destination directory and overwrite existing files
foreach ($file in $sourceFiles) {
  Copy-Item -Path $file -Destination $destinationDir -Force
}
