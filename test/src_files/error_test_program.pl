$PL Program: Linear Search
begin
    const n = 10; integer array A[n];
    integer x, i; Boolean found;
    proc Searché
    begin integer m;
        i,m :-= 1,n;
        do i < m ->
            if A[i] = x -> m := i; []
                ~(A[i] = x) -> i := i+1;
            fi;
        od; à
        found := A[i] = x;
    end; ?
    $ Input table!
    i := 1x;
    do ~(i > n) -> read A[i]; 
        i:= i+1; # some stuff
    od;
    $Test search: français ""''
    français ""'' ?? -- multiple errors
    "
    rea?d x;
    1x2x32
    do ~(x = 0) -> 
        call Search;
        if found -> write x, i; []
            ~found -> write x;
        fi;
        read x;
    od;
end.123notanum
