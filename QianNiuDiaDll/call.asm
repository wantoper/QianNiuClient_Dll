.code


calc proc
    sub rsp, 28h
    call rax
    add rsp, 28h
    ret
calc endp

end