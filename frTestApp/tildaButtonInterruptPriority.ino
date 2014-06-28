void tildaButtonInterruptPriority() {
    // reset pin interrupt handler IRQn priority levels to allow use of FreeRTOS API calls
    NVIC_DisableIRQ(PIOA_IRQn);
    NVIC_ClearPendingIRQ(PIOA_IRQn);
    NVIC_SetPriority(PIOA_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(PIOA_IRQn);

    NVIC_DisableIRQ(PIOB_IRQn);
    NVIC_ClearPendingIRQ(PIOB_IRQn);
    NVIC_SetPriority(PIOB_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(PIOB_IRQn);

    NVIC_DisableIRQ(PIOC_IRQn);
    NVIC_ClearPendingIRQ(PIOC_IRQn);
    NVIC_SetPriority(PIOC_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(PIOC_IRQn);

    NVIC_DisableIRQ(PIOD_IRQn);
    NVIC_ClearPendingIRQ(PIOD_IRQn);
    NVIC_SetPriority(PIOD_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(PIOD_IRQn);
}