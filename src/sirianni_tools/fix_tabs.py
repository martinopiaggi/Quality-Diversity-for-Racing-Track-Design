import os

def fix_indentation(filename):
    with open(filename, 'r') as f:
        content = f.read()
    
    # Convert tabs to spaces (4 spaces per tab)
    content = content.expandtabs(4)
    
    with open(filename, 'w', newline='\n') as f:
        f.write(content)

# Fix all Python files in directory
for file in os.listdir('.'):
    if file.endswith('.py'):
        fix_indentation(file)