/*
$Id$

gcc const_pointer.c -o const_pointer && ./const_pointer

*/

int main(void)
{
    int a = 10;
    const int* ptr1 = &a;
    //*ptr = 5; // wrong
    ptr1++;    // right


    int *const ptr2 = &a;
    *ptr2 = 5; // right
    //ptr2++;    // wrong

    const int *const ptr3 = &a;
    //*ptr3 = 5; // wrong
    //ptr3++;    // wrong
}

// Constant pointer vs Pointer to constant
// https://stackoverflow.com/questions/21476869/constant-pointer-vs-pointer-to-constant