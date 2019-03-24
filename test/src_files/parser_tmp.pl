$PL Program: Linear Search
begin
    integer array A, C, D, E[n];
    const n = 10; integer array A[n];
    integer x, i; Boolean found;
    proc Search
    begin integer m;
        i,m := 1,n;
        do i < m ->
            if A[i] = x -> m := i; []
                ~(A[i] = x) -> i := i+1;
            fi;
        od;
        found := A[i] = x;
    end;
    $ Input table
    i := 1;
    do ~(i > n) -> read A[i]; 
        i:= i+1; 
    od;
    $Test search:
    read x;
    do ~(x = 0) -> 
        call Search;
        if found -> write x, i; []
            ~found -> write x;
        fi;
        read x;
    od;
end.
