$Program simple; a sample program written in the language PL
begin
    integer x;
    x := 0;
    do (x < 10) -> x := x + 1;
    od;
end.