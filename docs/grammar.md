$$
\begin{aligned}
[statement] \to \begin{cases}
        [let]^* \\
        [return] \\
        [expression]^* \\
    \end{cases} \\
    [return] \to \begin{cases}
        [expression] \\
        \text{numeric literal} \\
    \end{cases} \\
    [let] \to
    \begin{cases}
        [expression] \\
        \text{numeric literal} \\
    \end{cases} \\
    [expression] \to \begin{cases}
        [variable] \\
        [multiplication] \\
        [division] \\
        [addition] \\
        [subtraction] \\
        \text{numeric literal} \\
    \end{cases}
\end{aligned}
$$