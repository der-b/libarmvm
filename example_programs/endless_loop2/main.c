int main(void)
{
    while(1) {
        __asm__("nop");
        for(int i = 0; i < 10; ++i) {
            __asm__("nop");
        }
        __asm__("nop");
    }
    return 0;
}
