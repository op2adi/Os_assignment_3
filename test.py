for _ in range(int(input())):
    b = list(map(int,input().split()))
    c = list(map(int,input().split()))
    q = list(map(int,input().split()))
    d = max(c)
    for i in q:
        q = 2**i
        r = 2**(i-1)
        if q>d:
            continue
        for j in range(len(c)):
            if c[j]%q==0:
                # if (c[j]+r>max)
                c[j]+=r
                if c[j]>d:
                    d=c[j]
                
    print(*c)
            