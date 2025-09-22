#include<stdio.h>
 #include<stdlib.h>
 int len,max,curr;
 void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
 }
 void sort(int *arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (*(arr + j) > *(arr + j + 1)) {
                swap((arr + j), (arr + j + 1));
            }
        }
    }
 }
 void fcfs(int str[])
 {
    int arr[len+1];
    arr[0]=curr;
    int k=1,seektime=0;
    for(int i=0;i<len;i++)
    {
        arr[k++]=str[i];
    }
    for(int i=0;i<k;i++)
    {
        printf("%d\t",arr[i]);
        if(i==0)
        {
           continue;
        }
        else
        {
            seektime+=abs(arr[i]-arr[i-1]);
        }
    }
    printf("\nTotal seek time=%d\n",seektime);
 }
 void scan(int str[])
 {
    int arr[len+1],k=0;
    arr[k++]=curr;
    int seektime=0;
    for(int i=0;i<len;i++)
    {
        if(str[i]>curr)
        {
            arr[k++]=str[i];
        }
    }
    arr[k]=max;
    sort(arr,k);
    for(int i=0;i<=k;i++)
    {
        printf("%d\t",arr[i]);
    }
    k=0;
    for(int i=0;i<len;i++)
    {
        if(str[i]<curr)
        {
            arr[k++]=str[i];
        }
    }
    sort(arr,k);
    int min=arr[0];
    for(int i=k-1;i>=0;i--)
    {
        printf("%d\t",arr[i]);
    }
    seektime+=(max-curr)+(max-min);
    printf("\nTotal seek time=%d\n",seektime);
    
}
 void cscan(int str[])
 {
    int arr[len+1],k=0;
    arr[k++]=curr;
    int seektime=0;
    for(int i=0;i<len;i++)
    {
        if(str[i]>curr)
        {
            arr[k++]=str[i];
        }
    }
    arr[k]=max;
    sort(arr,k);
    for(int i=0;i<=k;i++)
    {
        printf("%d\t",arr[i]);
    }
    k=0;
    for(int i=0;i<len;i++)
    {
        if(str[i]<curr)
        {
            arr[k++]=str[i];
        }
    }
    arr[k++]=0;
    sort(arr,k);
    int min=arr[0];
    for(int i=0;i<k;i++)
    {
        printf("%d\t",arr[i]);
    }
    seektime+=(max-curr)+(max-0)+(arr[--k]-0);
    printf("\nTotal seek time=%d\n",seektime);
    
}
 int main()
 {
   printf("Enter number of tracks-");
   scanf("%d",&len);
   int str[len];
   printf("Enter order of sequence-");
   for(int i=0;i<len;i++)
   {
    scanf("%d",&str[i]);
   }
   printf("Enter the maximum track-");
   scanf("%d",&max);
   printf("Enter the current position of header-");
   scanf("%d",&curr);
   printf("\nFCFS\n");
   fcfs(str);
   printf("\nSCAN\n");
   scan(str);
   printf("\nCSCAN\n");
   cscan(str);
 }
