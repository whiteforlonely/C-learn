#include <iostream>
#include <conio.h>

//int main() 
//{
//	int w, c, y, m, d;
//	printf("年月日为");
//	scanf_s("%d,%d,%d", &y, &m, &d);
//	c = y / 100;
//	while (y < 1582) {
//		while (m < 10) {
//			while (d < 4) {
//				if (m == 1 || m == 2) {
//					m = m + 12;
//					y = y - 1;
//				}
//				w = (y + (y / 4) + (y / 4) - 2 * c + ((13 * (m + 1)) / 5) + d + 2) % 7;
//				break;
//			}
//		}
//	}
//
//	if (y > 1582) {
//		if (m > 10) {
//			if (d > 4) {
//				if (m == 1 || m == 2) {
//					m = m + 12;
//					y = y - 1;
//				}
//				w = ((c / 4) - 2 * c + (y / 4) + ((13 * (m + 1)) / 5) + d - 1) % 7;
//				
//				break;
//			}
//		}
//	}
//	printf("为星期%d", w);
//	
//}