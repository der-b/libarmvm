#include <armvm_config.h>
#include <string.h>

int armvm_config_init(struct armvm_config *config, int argc, char **argv)
{
    memset(config, 0, sizeof(*config));
    return ARMVM_CONFIG_SUCCESS;
}


void armvm_config_usage(int argc, char **argv)
{
}


int armvm_config_cleanup(struct armvm_config *config)
{
    return ARMVM_CONFIG_SUCCESS;
}
