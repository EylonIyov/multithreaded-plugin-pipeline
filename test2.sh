#!/bin/bash

cd /workspaces/OS\ Final\ Task

echo "Rebuilding with debug output..."

for plugin_name in logger uppercaser rotator flipper expander typewriter; do 
    gcc -fPIC -shared -o output/${plugin_name}.so plugins/${plugin_name}.c plugins/plugin_common.c plugins/sync/monitor.c plugins/sync/consumer_producer.c -ldl -lpthread || { 
        echo "Failed to build $plugin_name" 
        exit 1 
    }    
done

gcc -ldl main.c -o output/analyzer

cd output

echo "=== Testing Bounded Queue with Debug Output ==="
echo "Queue size 1 - should see blocking:"
echo -e "item1\nitem2\nitem3\n<END>" | ./analyzer 1 logger rotator




echo -e "\nSingle plugin (no inter-plugin blocking):"
echo -e "single1\nsingle2\nsingle3\nsingle4\n<END>" | ./analyzer 1 typewriter uppercaser flipper logger

echo "=== Testing edge case <end> ==="
echo -e "single1\nsingle2\nsingle3\n<end>\nsingle4\n<END>" | ./analyzer 3 typewriter uppercaser logger


cd ../