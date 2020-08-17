#include <stdio.h>
#include <armvm_config.h>
#include <armvm.h>

int main(int argc, char **argv)
{
    struct armvm_opts opts;
    struct armvm armvm;
    struct armvm_config conf;

    if (armvm_config_init(&conf, argc, argv)) {
        armvm_config_usage(argc, argv);
        fprintf(stderr, "ERR: armvm_config_init() faild.\n");
        goto err;
    }

    if (armvm_opts_init(&opts)) {
        fprintf(stderr, "ERR: armvm_opts_init() faild.\n");
        goto err_conf;
    }

    if (armvm_start(&armvm, &opts)) {
        fprintf(stderr, "ERR: armvm_start() faild.\n");
        goto err_conf;
    }

    if (armvm_config_cleanup(&conf)) {
        fprintf(stderr, "ERR: armvm_config_cleanup() faild.\n");
        goto err;
    }

    return 0;
err_conf:
    if (armvm_config_cleanup(&conf)) {
        fprintf(stderr, "WARN: armvm_config_cleanup() faild.\n");
    }
err:
    return 1;
}
