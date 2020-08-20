#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <armvm_config.h>
#include <armvm.h>
#include <cli_version.h>

#define DEVICE_ID "STM32F070CB"

int main(int argc, char **argv)
{
    int ret_val = 0;
    struct armvm_opts opts;
    struct armvm armvm;
    struct armvm_config conf;

    if (0 != strcmp(VERSION, armvm_version())) {
        fprintf(stderr, "ERR: Version of the command line interface does not match the version libarmvm.so.\n");
        fprintf(stderr, "cli version: %s\n", VERSION);
        fprintf(stderr, "lib version: %s\n", armvm_version());
        ret_val = 1;
        goto err;
    }

    if (armvm_config_init(&conf, argc, argv)) {
        armvm_config_usage(argc, argv);
        ret_val = 1;
        goto err;
    }

    if (conf.show_help) {
        armvm_config_usage(argc, argv);
        ret_val = 0;
        goto err_conf;
    }

    if (conf.show_version) {
        printf("cli version     : %s\n", VERSION);
        printf("libarmvm version: %s\n", armvm_version());
        ret_val = 0;
        goto err_conf;
    }

    if (!conf.program) {
        fprintf(stderr, "ERROR: You need to provide a program which shall be loaded to the virtual machine (use --program).\n");
        ret_val = 1;
        goto err_conf;
    }

    if (armvm_opts_init(&opts)) {
        fprintf(stderr, "ERROR: armvm_opts_init() faild.\n");
        ret_val = 1;
        goto err_conf;
    }

    opts.isa = conf.isa;
    opts.program_file = conf.program;
    conf.program = NULL;
    opts.program_address = conf.program_address;
    opts.steps = conf.steps;

    // we currently only suppart one device
    opts.device_id = malloc(sizeof(DEVICE_ID));
    if (!opts.device_id) {
        fprintf(stderr, "ERROR: not enough memory.\n");
        ret_val = 1;
        goto err_opts;
    }

    memcpy(opts.device_id, DEVICE_ID, sizeof(DEVICE_ID));

    if (armvm_start(&armvm, &opts)) {
        fprintf(stderr, "ERROR: armvm_start() faild.\n");
        ret_val = 1;
        goto err_opts;
    }

err_opts:
    if (armvm_opts_cleanup(&opts)) {
        fprintf(stderr, "WARN: armvm_opts_cleanup() faild.\n");
    }

err_conf:
    if (armvm_config_cleanup(&conf)) {
        fprintf(stderr, "WARN: armvm_config_cleanup() faild.\n");
    }
err:
    return ret_val;
}
