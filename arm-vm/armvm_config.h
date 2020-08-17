#ifndef __ARMVM_CONFIG_H__

/** @file */

#define ARMVM_CONFIG_SUCCESS ( 0)
#define ARMVM_CONFIG_FAIL    (-1)

/**
 * @brief This struct contains the current configuration of the arm virtual machine.
 * The configuration my come from different sources such as command line options,
 * configuration fails or environment variables.
 */
struct armvm_config {
};

/**
 * @brief Loads the configuration from all possible sources.
 * The content of *armvm_config will be overwritten.
 * If an error occurs, this function prints an error message to stderr.
 *
 * @param armvm_config Pointer to a destination memory location, which will hold the configuration.
 * @param argc The argc argument provided to the main function.
 * @param argv The argv argument provided to the main function.
 * @return ARMVM_CONFIG_SUCCESS on success.
 */
int armvm_config_init(struct armvm_config *config, int argc, char **argv);


/**
 * @brief Prints the usage message to stdout.
 */
void armvm_config_usage(int argc, char **argv);


/**
 * @brief Cleans up a configuration and frees allocated memory.
 *
 * @param armvm_config Pointer to the configuration.
 * @return ARMVM_CONFIG_SUCCESS on success.
 */
int armvm_config_cleanup(struct armvm_config *config);

#endif
