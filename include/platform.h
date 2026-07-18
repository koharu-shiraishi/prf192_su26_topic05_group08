#ifndef PC_PLATFORM_H
#define PC_PLATFORM_H

#include <stdio.h>
#include <stdlib.h>

/* Operating system detection flags */
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#define WINDOWS_PLATFORM 1
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#define UNIX_PLATFORM 1
#endif

/* Terminal clear screen process */
static inline void platform_clearScreen(void)
{
#if defined(WINDOWS_PLATFORM)
    system("cls"); /* Windows OS Command Prompt / PowerShell */
#elif defined(UNIX_PLATFORM)
    system("clear"); /* Linux-based OS / macOS command */
#else
    /* Fallback: print multiple blank lines to push text up */
    for (int i = 0; i < 50; i++)
    {
        printf("\n");
    }
#endif
}

/* Pause screen processing safely across all platforms */
static inline void platform_pause(void)
{
    printf("\x1b[35m Press enter to continue...\x1b[0m");
    fflush(stdout);

    /*
     * Bước 1: Kiểm tra và làm sạch bộ nhớ đệm stdin nếu có ký tự thừa bên trong
     * (thường xảy ra sau khi dùng scanf hoặc fgets trước đó).
     */
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ; /* Đọc bỏ tất cả ký tự rác cho đến khi gặp \n hoặc EOF */

    /*
     * Bước 2: Sau khi stdin đã sạch, chờ người dùng nhấn phím Enter thực sự.
     * (Lưu ý: Nếu c == '\n' ở trên có nghĩa là stdin vừa có ký tự rác kèm Enter cũ,
     * ta vẫn gọi getchar() để chờ cú nhấn Enter mới từ người dùng).
     */
    if (c != EOF)
    {
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }
}

#endif /* PC_PLATFORM_H */