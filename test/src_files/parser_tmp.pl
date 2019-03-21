$PL Program: Linear Search
begin
    const n = 10; integer array A[n];
    integer x, i; Boolean found;
    proc Search
    begin integer m;
        i,m := 1,n;
        do i < m ->
            if A[i] = x -> m := i; []
                ~(A[i] = x) -> i := i+1;
        