#include <stdio.h> // Include the standard input/output library
#include <stdlib.h> // Include the standard library (for memory allocation, etc.)
#include <string.h> // Include the string library (for string manipulation)
#include <stdbool.h> // Include the boolean library (for true/false values)

#define MAXLINELEN 32767 // Define a maximum line length for reading the file
#define TIMESTAMP_LEN 17 // Define the standard length of a timestamp
#define BUILD_DATE "2024-07-10" // A macro to store the build date of the program

// Function prototypes (declarations) to tell the compiler about the functions
char** read_file_lines(const char *filename, int *num_lines);
void process_subtitles(char **lines, int *num_lines);
void remove_duplicate_lines(char **lines, int *num_lines);
void remove_duplicate_timestamps(char **lines, int *num_lines);
void write_srt_file(const char *filename, char **lines, int num_lines);
char* get_input_filename(int argc, char** argv);
char* get_output_filename(int argc, char** argv, const char *input_filename);

/*
 * Main function: Entry point of the program.
 * The program execution begins here.
 * 
 * argc: Number of command line arguments.
 * argv: Array of strings containing command line arguments.
 */
int main(int argc, char** argv) {
    char *filein, *fileout; // Pointers to store input and output filenames
    char **subtitles; // Pointer to an array of strings to store subtitle lines
    int num_lines = 0; // Variable to keep track of the number of subtitle lines

    // Print program information
    printf("Convert Youtube's Autotranscribed VTT to SRT [build %s]\n"
           "Converts .vtt files from youtube to .srt files.\n"
           "Usage: vtttosrt.exe [sourcesubtitles.vtt] [targetsubtitles.srt]\n"
           "Press Ctrl+C to abort.\n", BUILD_DATE);

    // 1. Get input filename
    filein = get_input_filename(argc, argv); // Get the input filename from user or arguments
    if (filein == NULL) { 
        return 1; // Error getting filename 
    }

    // 2. Get output filename
    fileout = get_output_filename(argc, argv, filein); // Get the output filename
    if (fileout == NULL) {
        free(filein); // Release memory allocated for the input filename
        return 1; // Error getting filename
    }

    // 3. Read subtitles from the input VTT file
    subtitles = read_file_lines(filein, &num_lines); 
    if (subtitles == NULL) {
        free(filein); // Release allocated memory
        free(fileout); 
        return 1; 
    }

    // 4. Process the subtitles (remove tags, duplicates)
    process_subtitles(subtitles, &num_lines); 

    // 5. Write the processed subtitles to the output SRT file
    write_srt_file(fileout, subtitles, num_lines);

    // 6. Clean up: Free allocated memory
    for (int i = 0; i < num_lines; i++) {
        free(subtitles[i]); // Free memory for each individual subtitle line
    }
    free(subtitles); // Free memory for the array of subtitle lines
    free(filein);  // Free memory for input filename
    free(fileout); // Free memory for output filename
    
    return 0; // Indicate successful program execution
}

/*
 * Reads the contents of a file and splits it into lines.
 *
 * filename: The name of the file to read.
 * num_lines: Pointer to an integer to store the number of lines read.
 *
 * Returns: A dynamically allocated array of strings, where each string is a line 
 * from the file. Returns NULL if there is an error.
 */
char** read_file_lines(const char *filename, int *num_lines) {
    FILE *fh; // File pointer to handle the file
    char *buffer; // Pointer to a character buffer to store the file contents
    char **lines; // Pointer to an array of strings (to store lines)
    long numbytes; // Variable to store the size of the file
    size_t bytes_read; // Variable to store the number of bytes read from the file

    // Open the file for reading
    fh = fopen(filename, "r"); 
    if (fh == NULL) {
        fprintf(stderr, "File %s not found.\n", filename); // Print error message to standard error
        return NULL; // Return NULL to indicate file opening error
    }

    // Get the size of the file
    fseek(fh, 0L, SEEK_END); // Move file pointer to the end of the file
    numbytes = ftell(fh); // Get the current position of the file pointer (which is the file size)
    fseek(fh, 0L, SEEK_SET); // Move file pointer back to the beginning of the file

    // Allocate memory to store the file contents
    buffer = (char*)calloc(numbytes + 1, sizeof(char)); // Allocate memory for the buffer 
    if (buffer == NULL) {
        fclose(fh); // Close the file
        fprintf(stderr, "Memory allocation error.\n"); 
        return NULL; 
    }

    // Read the file contents into the buffer
    bytes_read = fread(buffer, sizeof(char), numbytes, fh); 
    fclose(fh); // Close the file
    if (bytes_read != numbytes) { 
        free(buffer); // Free the buffer if there's an error
        fprintf(stderr, "Error reading file %s.\n", filename); 
        return NULL; 
    }

    // Allocate memory for an array of strings (to store lines)
    // We estimate an initial size, and will reallocate if needed
    lines = (char**)calloc(numbytes / 40, sizeof(char*));
    if (lines == NULL) {
        free(buffer); 
        fprintf(stderr, "Memory allocation error.\n");
        return NULL;
    }

    // Split the buffer into lines and store them in the 'lines' array
    lines[0] = &buffer[0]; // The first line starts at the beginning of the buffer
    *num_lines = 1; // Initialize line count to 1 (we've already got the first line)
    for (int i = 0; i < numbytes; i++) {
        if (buffer[i] == '\n') { // If we find a newline character
            buffer[i] = '\0'; // Replace newline with null terminator to create a C-style string
            
            // Reallocate memory for the 'lines' array if needed 
            if (*num_lines % 100 == 0) { // Reallocate every 100 lines 
                lines = (char**)realloc(lines, (*num_lines + 100) * sizeof(char*));
                if (lines == NULL) { 
                    free(buffer); 
                    fprintf(stderr, "Memory allocation error.\n");
                    return NULL; 
                }
            }
            lines[(*num_lines)++] = &buffer[i + 1]; // Store the next line
        }
    }
    return lines; // Return the array of lines
}


/*
 * Processes the subtitle lines to prepare them for SRT format.
 *
 * lines: Array of strings representing the subtitle lines.
 * num_lines: Pointer to an integer representing the number of lines.
 */
void process_subtitles(char **lines, int *num_lines) {
    // 1. Remove unnecessary tags and format timestamps
    int j = 0; // Index for the 'lines' array after processing
    for (int i = 0; i < *num_lines; i++) { // Iterate through each line
        char *line = lines[i]; // Get the current line
        if (strlen(line) > 1) { // If the line is not empty 
            lines[j] = line; // Copy the line to the new position (j)

            // Check if the line contains a timestamp
            char *ptr = strstr(line, " --> "); 
            if (ptr != NULL) {  // If a timestamp is found
                // Convert '.' to ',' in timestamps (SRT format)
                ptr[TIMESTAMP_LEN] = '\0'; // Add null terminator after timestamp
                int l = strlen(line);
                for (int m = 0; m <= l; m++) {
                    if (line[m] == '.') { 
                        line[m] = ','; 
                    }
                }
            } else {
                // If the line is not a timestamp, remove HTML-like tags 
                int l = strlen(line), k = 0;
                bool erase = false; 
                for (int m = 0; m <= l; m++) {
                    if (line[m] == '<') { 
                        erase = true; // Start erasing characters within tags
                    } else if (line[m] == '>') {
                        erase = false; // Stop erasing characters 
                    } else if (!erase) {
                        line[k++] = line[m]; // Copy characters if not within tags
                    }
                }
                line[k] = '\0'; // Add null terminator to the end of the processed line
            }
            j++; // Move to the next line after processing
        }
    }
    *num_lines = j; // Update the number of lines after removing empty/unnecessary ones

    // 2. Remove duplicate subtitle lines
    remove_duplicate_lines(lines, num_lines);
    
    // 3. Remove duplicate timestamp lines
    remove_duplicate_timestamps(lines, num_lines);
}

/*
 * Removes duplicate text lines from the subtitles.
 *
 * lines: Array of strings representing the subtitle lines.
 * num_lines: Pointer to an integer representing the number of lines.
 */
void remove_duplicate_lines(char **lines, int *num_lines) {
    int i = 0, j = 1;
    while ((strstr(lines[i], " --> ") == NULL) && (i < *num_lines)) {
        i++;
    }
    char *line = lines[0] = lines[i];
    i++;
    for (; i < *num_lines; i++) {
        char *ptr = strstr(lines[i], " --> ");
        bool erase = (strcmp(lines[i], line) == 0);
        if (ptr == NULL && erase) continue;
        if (ptr == NULL) line = lines[i];
        lines[j++] = lines[i];
    }
    *num_lines = j;
}

/*
 * Removes duplicate timestamp lines from the subtitles.
 *
 * lines: Array of strings representing the subtitle lines.
 * num_lines: Pointer to an integer representing the number of lines.
 */
void remove_duplicate_timestamps(char **lines, int *num_lines) {
    bool previousline = false;
    int j = 1;
    for (int i = 1; i < *num_lines; i++) {
        char *ptr = strstr(lines[i], " --> ");
        if (previousline || ptr == NULL) {
            lines[j++] = lines[i];
        }
        previousline = (ptr != NULL);
    }
    *num_lines = j;
}

/*
 * Writes the processed subtitles to an SRT file.
 *
 * filename: The name of the SRT file to write.
 * lines: Array of strings representing the subtitle lines.
 * num_lines: Number of lines in the 'lines' array.
 */
void write_srt_file(const char *filename, char **lines, int num_lines) {
    FILE *fh = fopen(filename, "w"); // Open the file for writing
    if (fh == NULL) { // Error handling: If the file cannot be opened
        fprintf(stderr, "File creating error. Sending the result to stdout.\n");
        fh = stdout; // Write to standard output as a fallback
    }

    int j = 1; // Counter for subtitle numbers
    for (int i = 0; i < num_lines;) {
        // Check for a timestamp line
        if (strstr(lines[i], " --> ") != NULL) { 
            // Check if a subtitle line follows the timestamp
            if (i + 1 < num_lines && strlen(lines[i + 1]) > 0) {
                // Write the subtitle number, timestamp, and subtitle text to the file
                fprintf(fh, "%d\n%s\n%s\n\n", j++, lines[i], lines[i + 1]);
                i += 2; // Move to the next timestamp (skip the subtitle line)
            } else {
                // If no subtitle line follows, skip the current timestamp line
                i++; 
            }
        } else {
            // If it's not a timestamp line, skip it 
            i++;
        }
    }

    if (fh != stdout) {
        fclose(fh); // Close the file if it's not standard output
    }
    printf("Subtitles exported to %s.\n", filename);
}

/*
 * Gets the input filename from either the command line arguments
 * or by prompting the user for input. 
 *
 * argc: Number of command line arguments.
 * argv: Array of strings containing command line arguments.
 *
 * Returns: Pointer to the input filename string. 
 */
char* get_input_filename(int argc, char** argv) {
    char *filein = (char*)malloc(MAXLINELEN * sizeof(char)); // Allocate memory for the filename
    if (filein == NULL) {
        fprintf(stderr, "Memory allocation error!\n");
        return NULL;
    }

    // Check if a filename was provided as a command line argument
    if (argc > 1) { 
        strncpy(filein, argv[1], MAXLINELEN - 1); // Copy filename from arguments
        filein[MAXLINELEN - 1] = '\0'; // Ensure null termination
    } else {
        // If no argument is provided, prompt the user for the filename
        printf("Enter input filename (*.vtt): ");
        if (fgets(filein, MAXLINELEN, stdin) == NULL) { // Read the filename from user input
            fprintf(stderr, "Error reading input filename.\n");
            free(filein);
            return NULL; 
        }
        filein[strlen(filein) - 1] = '\0'; // Remove trailing newline from fgets input
    }

    // Ensure the filename has the '.vtt' extension
    int k = strlen(filein);
    if (k == 0) { 
        free(filein); 
        return NULL; // Empty filename 
    } 
    if (k <= 3 || (strstr(&filein[k - 4], ".vtt") == NULL)) {
        strncat(filein, ".vtt", MAXLINELEN - k - 1); // Add .vtt extension if needed
    }
    return filein;
}

/*
 * Gets the output filename, either from command line arguments,
 * by automatically generating it from the input filename,
 * or by prompting the user. 
 *
 * argc: Number of command line arguments.
 * argv: Array of strings containing command line arguments.
 * input_filename: The input filename (used to generate a default output filename).
 *
 * Returns: Pointer to the output filename string.
 */
char* get_output_filename(int argc, char** argv, const char *input_filename) {
    char *fileout = (char*)malloc(MAXLINELEN * sizeof(char));  // Allocate memory for the filename
    if (fileout == NULL) {
        fprintf(stderr, "Memory allocation error!\n");
        return NULL;
    }

    if (argc == 3) {
        // Get output filename from command line argument
        strncpy(fileout, argv[2], MAXLINELEN - 1);
        fileout[MAXLINELEN - 1] = '\0'; 
    } else {
        // Generate a default output filename from the input filename 
        int k = strlen(input_filename); 
        strncpy(fileout, input_filename, MAXLINELEN - 1); // Copy the input filename
        fileout[k - 3] = 's'; // Replace 'vtt' with 'srt'
        fileout[k - 2] = 'r';
        fileout[k - 1] = 't';
        fileout[MAXLINELEN - 1] = '\0'; 

        // Prompt the user to accept the default or enter a new filename
        printf("Enter output filename (default: %s): ", fileout);
        if (fgets(fileout, MAXLINELEN, stdin) == NULL) {
            fprintf(stderr, "Error reading output filename.\n");
            free(fileout);
            return NULL;
        }
        fileout[strlen(fileout) - 1] = '\0'; // Remove trailing newline
    }

    // Ensure the filename has the '.srt' extension
    int k = strlen(fileout);
    if (k == 0) {
        strncpy(fileout, input_filename, MAXLINELEN - 1);
        fileout[MAXLINELEN - 1] = '\0'; 
        k = strlen(fileout);
    }
    if (k <= 3 || (strstr(&fileout[k - 4], ".srt") == NULL)) {
        strncat(fileout, ".srt", MAXLINELEN - k - 1); // Add .srt extension
    }
    return fileout;
}