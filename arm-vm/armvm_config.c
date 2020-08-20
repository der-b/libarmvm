#include <armvm_config.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <assert.h>

const struct option long_options[] = {
    {"program",         required_argument, 0, 'p'},
    {"address",         required_argument, 0, 'a'},
    {"isa",             required_argument, 0, 'i'},
    {"help",            no_argument,       0, 'h'},
    {"version",         no_argument,       0, 'v'},
    {0, 0, 0, 0}
};

const char short_options[] = "p:a:i:hv";

const char usage_message[] =
"-p, --program=FILE          Specifies the program, which shall be loaded by the vm.\n"
"-a, --address=ADDR          Memory location where the program shall be stored.\n"
"-i, --isa=ISA               Sets the instruction set architecture.\n"
"                            Valid values are: Armv6-M, Armv7-M, Armv8-M\n"
"-h, --help                  Display this help message and exit.\n"
"-v, --version               Display the version information and exit.\n"
"\n"
"ADDR arguments have to be an hexadecimal value starting with '0x'.\n";


int armvm_config_init(struct armvm_config *config, int argc, char **argv)
{
    int c;

    memset(config, 0, sizeof(*config));
    config->isa = ARMV6_M;
    config->program_address = 0x08000000;

    while(1) {
        int option_index = 0;
        c = getopt_long(argc, argv, short_options, long_options, &option_index);

        if (-1 == c) {
            break;
        }

        switch(c) {
            case 'h':
                config->show_help = 1;
                break;
            case 'v':
                config->show_version = 1;
                break;
            case 'i':
                config->isa = armvm_utils_string_to_isa(optarg);
                if (UNDEFINED_ISA == config->isa) {
                    fprintf(stderr, "ERROR: Unknown ISA: %s\n", optarg);
                }
                break;
            case 'p':
                {
                    const size_t len = strlen(optarg) + 1;
                    config->program = malloc(len * sizeof(char));
                    if (!config->program) {
                        fprintf(stderr, "ERROR: not enough memory.\n");
                        return ARMVM_CONFIG_FAIL;
                    }
                    strncpy(config->program, optarg, len);
                    break;
                }
            case 'a':
                {
                    // we assume that long long int is 64bit value
                    assert(sizeof(unsigned long long int) == 8);
                    assert(sizeof(uint64_t) == 8);
                    errno = 0;
                    uint64_t addr = strtoull(optarg, NULL, 16);
                    if (errno) {
                        fprintf(stderr, "ERROR: Argument to option -a/--address is invalid.\n", argv[option_index]);
                        return ARMVM_CONFIG_FAIL;
                    }
                    config->program_address = addr;
                }
                break;
            case '?':
                return ARMVM_CONFIG_FAIL;
            default:
                fprintf(stderr, "ERROR: Theoretical this is impossible\n", argv[option_index]);
                return ARMVM_CONFIG_FAIL;
        }
    }

    return ARMVM_CONFIG_SUCCESS;
}


void armvm_config_usage(int argc, char **argv)
{
    printf("Usage: %s [options]\n", argv[0]);
    printf("\n");
    printf("Options:\n");
    printf("%s\n", usage_message);
}


int armvm_config_cleanup(struct armvm_config *config)
{
    if (config->program) {
        free(config->program);
        config->program = NULL;
    }
    return ARMVM_CONFIG_SUCCESS;
}
