# stat_ls
Implementação do comando ls usando a syscall stat(), ao invés de statx()

Implementation of ls command using the stat() system call, instead of statx()

## Autor / Author
Marcelo A F Gomes

## Porquê? / Why?
Versões antigas de Docker / Rancher não implementam a syscall statx(), fazendo com que o comando ls não funcione de maneira apropriada dentro de contêineres Docker com versões de distros mais recentes.

Essa implementação espera dar uma sobrevida às instalações antigas de Docker / Rancher, de forma que não seja necessário fazer o upgrade de imediato apenas por esse problema. Até porque, em alguns casos, esse upgrade nem é viável.

---

Older versions of Docker / Rancher do not implement the statx() syscall, which makes the ls command to not function properly inside Docker containers wit newer distro versions.

This implementation hopes to give those older Docker / Rancher installations some slack before it'll be necessary to immediately upgrade just because of this problem. Besides, in some cases, this upgrade may not even be viable.
