/**
 * \file main.c
 * \brief Entrypoint for the PandOS+ kernel.
 *
 * \author Luca Tagliavini
 * \date 17-03-2022
 */

#include "init.h"

int main(int argc)
{
    init();
    return 0;
}
