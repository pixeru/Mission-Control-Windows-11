import os

directory = "."

for filename in os.listdir(directory):
    if filename.endswith(".h") or filename.endswith(".cpp"):
        filepath = os.path.join(directory, filename)
        try:
            with open(filepath, 'rb') as f:
                content = f.read()
            
            # Pattern for first line: #pragma once\n
            if b'#pragma once' in content:
                parts = content.split(b'#pragma once', 1)
                # Keep the pragma once and everything after the comment block
                suffix = parts[1]
                
                # Find the first non-comment line in suffixes
                lines = suffix.split(b'\n')
                new_suffix_lines = []
                stripping = True
                for line in lines:
                    stripped = line.strip()
                    if stripping:
                        if stripped.startswith(b'//') or not stripped:
                            continue
                        else:
                            stripping = False
                            new_suffix_lines.append(line)
                    else:
                        new_suffix_lines.append(line)
                
                print(f"Stripping header from {filename}")
                final_content = b'#pragma once\n' + b'\n'.join(new_suffix_lines)
                with open(filepath, 'wb') as f:
                    f.write(final_content)
            else:
                # Same logic but for files without #pragma once
                lines = content.split(b'\n')
                new_lines = []
                stripping = True
                for line in lines:
                    stripped = line.strip()
                    if stripping:
                        if stripped.startswith(b'//') or not stripped:
                            continue
                        else:
                            stripping = False
                            new_lines.append(line)
                    else:
                        new_lines.append(line)
                
                if len(new_lines) < len(lines):
                    print(f"Stripping header from {filename} (no pragma)")
                    final_content = b'\n'.join(new_lines)
                    with open(filepath, 'wb') as f:
                        f.write(final_content)

        except Exception as e:
            print(f"Error processing {filename}: {e}")
