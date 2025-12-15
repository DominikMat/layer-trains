import os

def consolidate_code(output_filename="layer_trains_codebase.txt"):
    """
    Searches the current directory and all subdirectories for .h and .cpp 
    files, and writes their titles and content into a single output file.
    
    Args:
        output_filename (str): The name of the file to write the content to.
    """
    
    # Get the directory where the script is being executed
    # os.getcwd() gives the current working directory
    root_dir = os.getcwd()
    
    # List of file extensions to search for
    target_extensions = ('.h', '.cpp', '.fs', '.gs', '.vs')

    # Start a file to write the consolidated content
    # 'w' mode (write) will create the file or overwrite it if it exists
    with open(output_filename, 'w', encoding='utf-8') as outfile:
        
        # os.walk generates the file names in a directory tree
        # by walking the tree either top-down or bottom-up.
        for dirpath, dirnames, filenames in os.walk(root_dir):
            
            # Exclude the directory where the output file is located 
            # to prevent the script from reading its own output in subsequent runs
            # if it were placed in a sub-folder.
            if dirpath == root_dir and output_filename in filenames:
                filenames.remove(output_filename)

            for filename in filenames:
                
                # Check if the file has a target extension
                if filename.lower().endswith(target_extensions):
                    
                    # Full path to the source file
                    filepath = os.path.join(dirpath, filename)
                    
                    # Create a readable header for the output file
                    relative_path = os.path.relpath(filepath, root_dir)
                    header = f"*** FILE: {relative_path} ***\n"
                    
                    # Write the header to the output file
                    outfile.write("=" * 70 + "\n")
                    outfile.write(header)
                    outfile.write("=" * 70 + "\n\n")

                    print(f"Adding: {relative_path}")
                    
                    try:
                        # Open the source file and read its content
                        # 'r' mode (read)
                        with open(filepath, 'r', encoding='utf-8') as infile:
                            content = infile.read()
                            
                        # Write the content to the output file
                        outfile.write(content)
                        
                        # Add a separator for the next file
                        outfile.write("\n\n") 
                        
                    except UnicodeDecodeError:
                        error_msg = f"\n!!! SKIPPING: Could not read {relative_path} due to encoding error (not UTF-8). !!!\n\n"
                        outfile.write(error_msg)
                        print(f"Skipped {relative_path} (encoding error)")
                    except Exception as e:
                        error_msg = f"\n!!! SKIPPING: An unexpected error occurred while reading {relative_path}: {e} !!!\n\n"
                        outfile.write(error_msg)
                        print(f"Skipped {relative_path} (unexpected error: {e})")

    print("\n" + "#" * 70)
    print(f"SUCCESS: All relevant files have been consolidated into '{output_filename}'.")
    print("#" * 70)

# Execute the function
if __name__ == "__main__":
    consolidate_code()