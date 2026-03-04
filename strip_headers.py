import os
import re

# Match the block of comments at the start of files
# This regex looks for blocks starting with // THIS CODE AND INFORMATION
# or the newly added Mission Control comments.
patterns = [
    r'// THIS CODE AND INFORMATION IS PROVIDED "AS IS"[\s\S]*?// Particular Purpose\.',
    r'// Mission Control for Windows 11 by pixeru[\s\S]*?// Extensive development by pixeru due to discontinued development of original project\.',
    r'// THIS CODE AND INFORMATION IS PROVIDED "AS IS"[\s\S]*?// YOU MAY NOT USE THIS CODE[\s\S]*?// Copyright \(c\) Emcee App Software\. All rights reserved',
    r'// THIS CODE AND INFORMATION IS PROVIDED "AS IS"[\s\S]*?//[\s]*\n'
]

directory = "."

for filename in os.listdir(directory):
    if filename.endswith(".h") or filename.endswith(".cpp"):
        filepath = os.path.join(directory, filename)
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
            
            # More direct approach: Remove all lines starting with // at the very beginning of the file
            # until we hit a non-comment line or a blank line that separates the header.
            lines = content.splitlines()
            if not lines:
                continue
                
            start_code_idx = 0
            for i, line in enumerate(lines):
                trimmed = line.strip()
                # Keep going as long as it's a comment line or empty
                if trimmed.startswith("//") or not trimmed:
                    start_code_idx = i + 1
                else:
                    # Found code (e.g. #include, #pragma)
                    break
            
            if start_code_idx > 0:
                print(f"Removing {start_code_idx} lines of header from {filename}")
                new_content = '\n'.join(lines[start_code_idx:])
                # Ensure we don't end up with a leading blank line if there was one after the header
                new_content = new_content.lstrip()
                
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(new_content)
        except Exception as e:
            print(f"Error processing {filename}: {e}")
