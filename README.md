
# VTT to SRT Converter

This C program converts subtitles from a VTT (WebVTT) file to an SRT (SubRip Subtitle) file. It is designed to handle YouTube's auto-transcribed VTT files and convert them into a more widely supported SRT format. The code is structured for readability and maintainability, making it easy to understand and modify. 

## Features

- **Accurate Conversion:** Converts VTT files to SRT format, maintaining correct subtitle timings and sequence.
- **Cleanup:** Removes unnecessary tags (like `<c>`, `</c>`) and formatting from YouTube's auto-generated subtitles.
- **Deduplication:**  Handles duplicate lines and timestamps to create a clean SRT output.
- **User-Friendly:** Provides a simple command-line interface for easy usage.
- **Robust Error Handling:** Includes checks for file operations, memory allocation, and other potential errors.

## Usage

### Compilation

To compile the program, use a C compiler like `gcc`:

```bash
gcc -o vtttosrt vtttosrt.c
```

### Running the Program

You can run the program with the following command:

```bash
./vtttosrt [input.vtt] [output.srt]
```

- **`input.vtt`:** The path to the input VTT file.
- **`output.srt`:** The path to the output SRT file (optional).

If the output file is not specified, the program will prompt you to enter it.

### Example

```bash
./vtttosrt example.vtt example.srt 
```

## How It Works

1. **Input and Output:** The program takes the input VTT filename and (optionally) the output SRT filename from the command line arguments or prompts the user.

2. **File Reading:** The program reads the entire contents of the input VTT file into memory.

3. **Line Processing:**
   - The file content is split into individual lines.
   - Each line is processed: 
     - Timestamps are extracted and converted to the SRT format (using commas instead of periods as decimal separators).
     - Unnecessary tags and formatting are removed.

4. **Duplicate Removal:**
   - The program identifies and removes duplicate subtitle lines.
   - Duplicate timestamp entries are also removed.

5. **File Writing:**  The processed subtitle lines are written to the specified output SRT file, adhering to the SRT format.

## Code Structure

The C code is organized into functions to improve modularity and readability:

- **`main`:** Handles program flow, calling other functions for specific tasks.
- **`read_file_lines`:** Reads the input file and returns an array of lines.
- **`process_subtitles`:** Orchestrates the main subtitle processing steps (removing tags, duplicates).
- **`remove_duplicate_lines`:** Removes duplicate text lines.
- **`remove_duplicate_timestamps`:** Removes duplicate timestamp lines.
- **`write_srt_file`:** Writes the final processed subtitles to the SRT file.
- **`get_input_filename`:** Gets the input filename (from arguments or user).
- **`get_output_filename`:**  Gets the output filename (from arguments or user). 

## Error Handling

The program includes error handling for:
- File opening and reading errors.
- Memory allocation errors. 
- Issues during file writing.

## License

This program is in the public domain. You can use, modify, and distribute it without any restrictions. 

## Acknowledgements

This program was inspired by the need to convert YouTube's auto-transcribed VTT files to SRT format for better compatibility with media players and subtitle editors.
