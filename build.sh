for plugin_name in logger uppercaser rotator flipper expander typewriter; do 
    print_status "Building plugin: $plugin_name" 
    gcc -fPIC -shared -o output/${plugin_name}.so \ 
        plugins/${plugin_name}.c \ 
        plugins/plugin_common.c \ 
        plugins/sync/monitor.c \ 
        plugins/sync/consumer_producer.c \ 
        -ldl -lpthread || { 
        print_error "Failed to build $plugin_name" 
        exit 1 
        }    
done 