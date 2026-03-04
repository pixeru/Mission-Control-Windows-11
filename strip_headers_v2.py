import os

directory = "."

for filename in os.listdir(directory):
    if filename.endswith(".h") or filename.endswith(".cpp"):
        filepath = os.path.join(directory, filename)
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
            
            lines = content.splitlines()
            if not lines:
                continue
            
            # Identify where the code actually starts
            first_code_line = -1
            for i, line in enumerate(lines):
                trimmed = line.strip()
                # If it's not a comment and not empty, it's code
                if trimmed and not trimmed.startswith("//"):
                    first_code_line = i
                    break
            
            if first_code_line != -1:
                # If the first code line is not the first line, we have a header to strip
                if first_code_line > 0:
                    print(f"Stripping {first_code_line} header lines from {filename}")
                    new_lines = lines[first_code_line:]
                    # Write back joined by \n
                    with open(filepath, 'w', encoding='utf-8') as f:
                        f.write('\n'.join(new_lines) + '\n')
            else:
                # If no code was found (unlikely), check if it's all comments
                if all(line.strip().startswith("//") or not line.strip() for line in lines):
                    print(f"File {filename} appears to be all comments, clearing it.")
                    with open(filepath, 'w', encoding='utf-8') as f:
                        f.write('')
                        
        except Exception as e:
            print(f"Error processing {filename}: {e}")
