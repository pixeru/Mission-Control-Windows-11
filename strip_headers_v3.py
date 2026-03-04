import os

directory = "."

for filename in os.listdir(directory):
    if filename.endswith(".h") or filename.endswith(".cpp"):
        filepath = os.path.join(directory, filename)
        try:
            with open(filepath, 'rb') as f:
                content = f.read()
            
            # Detect UTF-8 BOM
            bom = b'\xef\xbb\xbf'
            has_bom = content.startswith(bom)
            
            text = content.decode('utf-8', errors='ignore')
            lines = text.splitlines()
            
            if not lines:
                continue
            
            # Find the first line that is actual code
            first_code_line = -1
            for i, line in enumerate(lines):
                trimmed = line.strip()
                if trimmed and not trimmed.startswith("//"):
                    first_code_line = i
                    break
            
            if first_code_line != -1:
                print(f"Stripping {first_code_line} header lines from {filename}")
                new_text = '\n'.join(lines[first_code_line:]) + '\n'
                new_content = new_text.encode('utf-8')
                
                if has_bom:
                    new_content = bom + new_content
                
                with open(filepath, 'wb') as f:
                    f.write(new_content)
        except Exception as e:
            print(f"Error processing {filename}: {e}")
