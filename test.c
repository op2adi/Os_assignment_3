#include <stdio.h>
#include <sys/resource.h>

int main() {
    struct rlimit rlim;

    if (getrlimit(RLIMIT_AS, &rlim) == 0) {
        printf("Soft limit: %llu\n", (unsigned long long)rlim.rlim_cur);
        printf("Hard limit: %llu\n", (unsigned long long)rlim.rlim_max);
    } else {
        perror("getrlimit");
    }

    return 0;
}
