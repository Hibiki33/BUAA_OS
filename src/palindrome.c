#include <stdio.h>
int main() {
	int n;
	scanf("%d", &n);

	int nums[5];
    	int i = 0;
    	int j = 0;
    	int flag = 1;

    	while (n > 0) {
        	nums[i++] = n % 10;
        	n /= 10;
    	}

    	for (i--; j <= i; j++, i--) {
        	if (nums[i] != nums[j]) {
            		flag = 0;
            		break;
        	}
    	}

	if (flag) {
		printf("Y\n");
	} else {
		printf("N\n");
	}
	return 0;
}
