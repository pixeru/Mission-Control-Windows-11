import os

directory = "."

for filename in os.listdir(directory):
    if filename.endswith(".h") or filename.endswith(".cpp"):
        filepath = os.path.join(directory, filename)
        try:
            with open(filepath, 'rb') as f:
                content = f.read()
            
            # Use raw string checking for these common patterns
            patterns = [
                b'// THIS CODE AND INFORMATION IS PROVIDED "AS IS"',
                b'// Mission Control for Windows 11 by pixeru',
                b'// Source code originally by Emcee',
                b'// Extensive development by pixeru'
            ]
            
            lines = content.split(b'\n')
            new_lines = []
            stripping = True
            
            for line in lines:
                # Basic check for comment
                stripped_line = line.strip()
                # Handle potential BOM in the first line
                if stripped_line.startswith(b'\xef\xbb\xbf'):
                    stripped_line = stripped_line[3:].strip()
                
                if stripping:
                    if stripped_line.startswith(b'//') or not stripped_line:
                        # Continue stripping
                        continue
                    else:
                        # Found first non-comment, non-empty line
                        stripping = False
                        new_lines.append(line)
                else:
                    new_lines.append(line)
            
            if len(new_lines) < len(lines):
                print(f"Stripping {len(lines) - len(new_lines)} lines from {filename}")
                # Prepend BOM if original had it
                final_content = b'\n'.join(new_lines)
                if content.startswith(b'\xef\xbb\xbf'):
                     final_content = b'\xef\xbb\xbf' + final_content
                     
                with open(filepath, 'wb') as f:
                    f.write(final_content)
            else:
                print(f"No header stripping performed on {filename}")
        except Exception as e:
            print(f"Error processing {filename}: {e}")
