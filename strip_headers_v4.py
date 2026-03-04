import os

directory = "."

for filename in os.listdir(directory):
    if filename.endswith(".h") or filename.endswith(".cpp"):
        filepath = os.path.join(directory, filename)
        try:
            with open(filepath, 'rb') as f:
                content = f.read()
            
            # UTF-8 BOM is \xef\xbb\xbf
            bom = b'\xef\xbb\xbf'
            has_bom = content.startswith(bom)
            
            # Decode the text
            text = content.decode('utf-8', errors='ignore')
            lines = text.splitlines(keepends=True)
            
            if not lines:
                continue
            
            # Find the first line that is actual code (not a comment and not empty)
            first_code_line_idx = -1
            for i, line in enumerate(lines):
                trimmed = line.strip()
                # Skip BOM from the first line for check
                if i == 0 and trimmed.startswith('\ufeff'):
                    trimmed = trimmed[1:].strip()
                
                if trimmed and not trimmed.startswith("//"):
                    first_code_line_idx = i
                    break
            
            if first_code_line_idx != -1 and first_code_line_idx > 0:
                print(f"Stripping {first_code_line_idx} header lines from {filename}")
                # Take all lines from the first code line onwards
                new_text_lines = lines[first_code_line_idx:]
                new_text = "".join(new_text_lines)
                
                # Write back with original CRLF/LF and optional BOM
                new_content = new_text.encode('utf-8')
                if has_bom:
                    # If we accidentally included another BOM in the text, remove it
                    if new_content.startswith(bom):
                        pass # It's already there
                    else:
                        new_content = bom + new_content
                
                with open(filepath, 'wb') as f:
                    f.write(new_content)
            elif first_code_line_idx == 0:
                print(f"No header comments found in {filename}")
                
        except Exception as e:
            print(f"Error processing {filename}: {e}")
