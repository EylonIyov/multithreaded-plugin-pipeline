# Colors for output 
RED='\033[0;31m' 
GREEN='\033[0;32m' 
YELLOW='\033[1;33m' 
NC='\033[0m' # No Color 
 
# Function to print colored output 
print_status() 
{ 
    echo -e "${GREEN}[BUILD]${NC} $1" 
} 
 
print_warning() 
{ 
    echo -e "${YELLOW}[WARNING]${NC} $1" 
} 
 
print_error() 
{ 
    echo -e "${RED}[ERROR]${NC} $1" 
}

for plugin_name in logger uppercaser rotator flipper expander typewriter; do 
    print_status "Building plugin: $plugin_name" 
    gcc -fPIC -shared -o output/${plugin_name}.so plugins/${plugin_name}.c plugins/plugin_common.c plugins/sync/monitor.c plugins/sync/consumer_producer.c -ldl -lpthread || { 
        print_error "Failed to build $plugin_name" 
        exit 1 
    }    
done

print_status "Building analyzer"

gcc -ldl main.c -o output/analyzer

cd output

print_status "Testing pipeline with multiple inputs"

echo -e "hello\nworld\ntest input\nquick test\nmore data\n<END>" | ./analyzer 3 uppercaser logger rotator typewriter


cd ../