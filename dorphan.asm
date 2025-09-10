
user/_dorphan:     file format elf64-littleriscv


Disassembly of section .text:

0000000000000000 <main>:

char buf[BUFSZ];

int
main(int argc, char **argv)
{
   0:	1101                	addi	sp,sp,-32
   2:	ec06                	sd	ra,24(sp)
   4:	e822                	sd	s0,16(sp)
   6:	e426                	sd	s1,8(sp)
   8:	1000                	addi	s0,sp,32
  char *s = argv[0];
   a:	6184                	ld	s1,0(a1)

  if(mkdir("dd") != 0){
   c:	00001517          	auipc	a0,0x1
  10:	8f450513          	addi	a0,a0,-1804 # 900 <malloc+0xfa>
  14:	372000ef          	jal	386 <mkdir>
  18:	c919                	beqz	a0,2e <main+0x2e>
    printf("%s: mkdir dd failed\n", s);
  1a:	85a6                	mv	a1,s1
  1c:	00001517          	auipc	a0,0x1
  20:	8ec50513          	addi	a0,a0,-1812 # 908 <malloc+0x102>
  24:	72e000ef          	jal	752 <printf>
    exit(1);
  28:	4505                	li	a0,1
  2a:	2f4000ef          	jal	31e <exit>
  }

  if(chdir("dd") != 0){
  2e:	00001517          	auipc	a0,0x1
  32:	8d250513          	addi	a0,a0,-1838 # 900 <malloc+0xfa>
  36:	358000ef          	jal	38e <chdir>
  3a:	c919                	beqz	a0,50 <main+0x50>
    printf("%s: chdir dd failed\n", s);
  3c:	85a6                	mv	a1,s1
  3e:	00001517          	auipc	a0,0x1
  42:	8e250513          	addi	a0,a0,-1822 # 920 <malloc+0x11a>
  46:	70c000ef          	jal	752 <printf>
    exit(1);
  4a:	4505                	li	a0,1
  4c:	2d2000ef          	jal	31e <exit>
  }

  if (unlink("../dd") < 0) {
  50:	00001517          	auipc	a0,0x1
  54:	8e850513          	addi	a0,a0,-1816 # 938 <malloc+0x132>
  58:	316000ef          	jal	36e <unlink>
  5c:	00054d63          	bltz	a0,76 <main+0x76>
    printf("%s: unlink failed\n", s);
    exit(1);
  }
  printf("wait for kill and reclaim\n");
  60:	00001517          	auipc	a0,0x1
  64:	8f850513          	addi	a0,a0,-1800 # 958 <malloc+0x152>
  68:	6ea000ef          	jal	752 <printf>
  // sit around until killed
  for(;;) pause(1000);
  6c:	3e800513          	li	a0,1000
  70:	33e000ef          	jal	3ae <pause>
  74:	bfe5                	j	6c <main+0x6c>
    printf("%s: unlink failed\n", s);
  76:	85a6                	mv	a1,s1
  78:	00001517          	auipc	a0,0x1
  7c:	8c850513          	addi	a0,a0,-1848 # 940 <malloc+0x13a>
  80:	6d2000ef          	jal	752 <printf>
    exit(1);
  84:	4505                	li	a0,1
  86:	298000ef          	jal	31e <exit>

000000000000008a <start>:
//
// wrapper so that it's OK if main() does not call exit().
//
void
start()
{
  8a:	1141                	addi	sp,sp,-16
  8c:	e406                	sd	ra,8(sp)
  8e:	e022                	sd	s0,0(sp)
  90:	0800                	addi	s0,sp,16
  int r;
  extern int main();
  r = main();
  92:	f6fff0ef          	jal	0 <main>
  exit(r);
  96:	288000ef          	jal	31e <exit>

000000000000009a <strcpy>:
}

char*
strcpy(char *s, const char *t)
{
  9a:	1141                	addi	sp,sp,-16
  9c:	e422                	sd	s0,8(sp)
  9e:	0800                	addi	s0,sp,16
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
  a0:	87aa                	mv	a5,a0
  a2:	0585                	addi	a1,a1,1
  a4:	0785                	addi	a5,a5,1
  a6:	fff5c703          	lbu	a4,-1(a1)
  aa:	fee78fa3          	sb	a4,-1(a5)
  ae:	fb75                	bnez	a4,a2 <strcpy+0x8>
    ;
  return os;
}
  b0:	6422                	ld	s0,8(sp)
  b2:	0141                	addi	sp,sp,16
  b4:	8082                	ret

00000000000000b6 <strcmp>:

int
strcmp(const char *p, const char *q)
{
  b6:	1141                	addi	sp,sp,-16
  b8:	e422                	sd	s0,8(sp)
  ba:	0800                	addi	s0,sp,16
  while(*p && *p == *q)
  bc:	00054783          	lbu	a5,0(a0)
  c0:	cb91                	beqz	a5,d4 <strcmp+0x1e>
  c2:	0005c703          	lbu	a4,0(a1)
  c6:	00f71763          	bne	a4,a5,d4 <strcmp+0x1e>
    p++, q++;
  ca:	0505                	addi	a0,a0,1
  cc:	0585                	addi	a1,a1,1
  while(*p && *p == *q)
  ce:	00054783          	lbu	a5,0(a0)
  d2:	fbe5                	bnez	a5,c2 <strcmp+0xc>
  return (uchar)*p - (uchar)*q;
  d4:	0005c503          	lbu	a0,0(a1)
}
  d8:	40a7853b          	subw	a0,a5,a0
  dc:	6422                	ld	s0,8(sp)
  de:	0141                	addi	sp,sp,16
  e0:	8082                	ret

00000000000000e2 <strlen>:

uint
strlen(const char *s)
{
  e2:	1141                	addi	sp,sp,-16
  e4:	e422                	sd	s0,8(sp)
  e6:	0800                	addi	s0,sp,16
  int n;

  for(n = 0; s[n]; n++)
  e8:	00054783          	lbu	a5,0(a0)
  ec:	cf91                	beqz	a5,108 <strlen+0x26>
  ee:	0505                	addi	a0,a0,1
  f0:	87aa                	mv	a5,a0
  f2:	86be                	mv	a3,a5
  f4:	0785                	addi	a5,a5,1
  f6:	fff7c703          	lbu	a4,-1(a5)
  fa:	ff65                	bnez	a4,f2 <strlen+0x10>
  fc:	40a6853b          	subw	a0,a3,a0
 100:	2505                	addiw	a0,a0,1
    ;
  return n;
}
 102:	6422                	ld	s0,8(sp)
 104:	0141                	addi	sp,sp,16
 106:	8082                	ret
  for(n = 0; s[n]; n++)
 108:	4501                	li	a0,0
 10a:	bfe5                	j	102 <strlen+0x20>

000000000000010c <memset>:

void*
memset(void *dst, int c, uint n)
{
 10c:	1141                	addi	sp,sp,-16
 10e:	e422                	sd	s0,8(sp)
 110:	0800                	addi	s0,sp,16
  char *cdst = (char *) dst;
  int i;
  for(i = 0; i < n; i++){
 112:	ca19                	beqz	a2,128 <memset+0x1c>
 114:	87aa                	mv	a5,a0
 116:	1602                	slli	a2,a2,0x20
 118:	9201                	srli	a2,a2,0x20
 11a:	00a60733          	add	a4,a2,a0
    cdst[i] = c;
 11e:	00b78023          	sb	a1,0(a5)
  for(i = 0; i < n; i++){
 122:	0785                	addi	a5,a5,1
 124:	fee79de3          	bne	a5,a4,11e <memset+0x12>
  }
  return dst;
}
 128:	6422                	ld	s0,8(sp)
 12a:	0141                	addi	sp,sp,16
 12c:	8082                	ret

000000000000012e <strchr>:

char*
strchr(const char *s, char c)
{
 12e:	1141                	addi	sp,sp,-16
 130:	e422                	sd	s0,8(sp)
 132:	0800                	addi	s0,sp,16
  for(; *s; s++)
 134:	00054783          	lbu	a5,0(a0)
 138:	cb99                	beqz	a5,14e <strchr+0x20>
    if(*s == c)
 13a:	00f58763          	beq	a1,a5,148 <strchr+0x1a>
  for(; *s; s++)
 13e:	0505                	addi	a0,a0,1
 140:	00054783          	lbu	a5,0(a0)
 144:	fbfd                	bnez	a5,13a <strchr+0xc>
      return (char*)s;
  return 0;
 146:	4501                	li	a0,0
}
 148:	6422                	ld	s0,8(sp)
 14a:	0141                	addi	sp,sp,16
 14c:	8082                	ret
  return 0;
 14e:	4501                	li	a0,0
 150:	bfe5                	j	148 <strchr+0x1a>

0000000000000152 <gets>:

char*
gets(char *buf, int max)
{
 152:	711d                	addi	sp,sp,-96
 154:	ec86                	sd	ra,88(sp)
 156:	e8a2                	sd	s0,80(sp)
 158:	e4a6                	sd	s1,72(sp)
 15a:	e0ca                	sd	s2,64(sp)
 15c:	fc4e                	sd	s3,56(sp)
 15e:	f852                	sd	s4,48(sp)
 160:	f456                	sd	s5,40(sp)
 162:	f05a                	sd	s6,32(sp)
 164:	ec5e                	sd	s7,24(sp)
 166:	1080                	addi	s0,sp,96
 168:	8baa                	mv	s7,a0
 16a:	8a2e                	mv	s4,a1
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
 16c:	892a                	mv	s2,a0
 16e:	4481                	li	s1,0
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
 170:	4aa9                	li	s5,10
 172:	4b35                	li	s6,13
  for(i=0; i+1 < max; ){
 174:	89a6                	mv	s3,s1
 176:	2485                	addiw	s1,s1,1
 178:	0344d663          	bge	s1,s4,1a4 <gets+0x52>
    cc = read(0, &c, 1);
 17c:	4605                	li	a2,1
 17e:	faf40593          	addi	a1,s0,-81
 182:	4501                	li	a0,0
 184:	1b2000ef          	jal	336 <read>
    if(cc < 1)
 188:	00a05e63          	blez	a0,1a4 <gets+0x52>
    buf[i++] = c;
 18c:	faf44783          	lbu	a5,-81(s0)
 190:	00f90023          	sb	a5,0(s2)
    if(c == '\n' || c == '\r')
 194:	01578763          	beq	a5,s5,1a2 <gets+0x50>
 198:	0905                	addi	s2,s2,1
 19a:	fd679de3          	bne	a5,s6,174 <gets+0x22>
    buf[i++] = c;
 19e:	89a6                	mv	s3,s1
 1a0:	a011                	j	1a4 <gets+0x52>
 1a2:	89a6                	mv	s3,s1
      break;
  }
  buf[i] = '\0';
 1a4:	99de                	add	s3,s3,s7
 1a6:	00098023          	sb	zero,0(s3)
  return buf;
}
 1aa:	855e                	mv	a0,s7
 1ac:	60e6                	ld	ra,88(sp)
 1ae:	6446                	ld	s0,80(sp)
 1b0:	64a6                	ld	s1,72(sp)
 1b2:	6906                	ld	s2,64(sp)
 1b4:	79e2                	ld	s3,56(sp)
 1b6:	7a42                	ld	s4,48(sp)
 1b8:	7aa2                	ld	s5,40(sp)
 1ba:	7b02                	ld	s6,32(sp)
 1bc:	6be2                	ld	s7,24(sp)
 1be:	6125                	addi	sp,sp,96
 1c0:	8082                	ret

00000000000001c2 <stat>:

int
stat(const char *n, struct stat *st)
{
 1c2:	1101                	addi	sp,sp,-32
 1c4:	ec06                	sd	ra,24(sp)
 1c6:	e822                	sd	s0,16(sp)
 1c8:	e04a                	sd	s2,0(sp)
 1ca:	1000                	addi	s0,sp,32
 1cc:	892e                	mv	s2,a1
  int fd;
  int r;

  fd = open(n, O_RDONLY);
 1ce:	4581                	li	a1,0
 1d0:	18e000ef          	jal	35e <open>
  if(fd < 0)
 1d4:	02054263          	bltz	a0,1f8 <stat+0x36>
 1d8:	e426                	sd	s1,8(sp)
 1da:	84aa                	mv	s1,a0
    return -1;
  r = fstat(fd, st);
 1dc:	85ca                	mv	a1,s2
 1de:	198000ef          	jal	376 <fstat>
 1e2:	892a                	mv	s2,a0
  close(fd);
 1e4:	8526                	mv	a0,s1
 1e6:	160000ef          	jal	346 <close>
  return r;
 1ea:	64a2                	ld	s1,8(sp)
}
 1ec:	854a                	mv	a0,s2
 1ee:	60e2                	ld	ra,24(sp)
 1f0:	6442                	ld	s0,16(sp)
 1f2:	6902                	ld	s2,0(sp)
 1f4:	6105                	addi	sp,sp,32
 1f6:	8082                	ret
    return -1;
 1f8:	597d                	li	s2,-1
 1fa:	bfcd                	j	1ec <stat+0x2a>

00000000000001fc <atoi>:

int
atoi(const char *s)
{
 1fc:	1141                	addi	sp,sp,-16
 1fe:	e422                	sd	s0,8(sp)
 200:	0800                	addi	s0,sp,16
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
 202:	00054683          	lbu	a3,0(a0)
 206:	fd06879b          	addiw	a5,a3,-48
 20a:	0ff7f793          	zext.b	a5,a5
 20e:	4625                	li	a2,9
 210:	02f66863          	bltu	a2,a5,240 <atoi+0x44>
 214:	872a                	mv	a4,a0
  n = 0;
 216:	4501                	li	a0,0
    n = n*10 + *s++ - '0';
 218:	0705                	addi	a4,a4,1
 21a:	0025179b          	slliw	a5,a0,0x2
 21e:	9fa9                	addw	a5,a5,a0
 220:	0017979b          	slliw	a5,a5,0x1
 224:	9fb5                	addw	a5,a5,a3
 226:	fd07851b          	addiw	a0,a5,-48
  while('0' <= *s && *s <= '9')
 22a:	00074683          	lbu	a3,0(a4)
 22e:	fd06879b          	addiw	a5,a3,-48
 232:	0ff7f793          	zext.b	a5,a5
 236:	fef671e3          	bgeu	a2,a5,218 <atoi+0x1c>
  return n;
}
 23a:	6422                	ld	s0,8(sp)
 23c:	0141                	addi	sp,sp,16
 23e:	8082                	ret
  n = 0;
 240:	4501                	li	a0,0
 242:	bfe5                	j	23a <atoi+0x3e>

0000000000000244 <memmove>:

void*
memmove(void *vdst, const void *vsrc, int n)
{
 244:	1141                	addi	sp,sp,-16
 246:	e422                	sd	s0,8(sp)
 248:	0800                	addi	s0,sp,16
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  if (src > dst) {
 24a:	02b57463          	bgeu	a0,a1,272 <memmove+0x2e>
    while(n-- > 0)
 24e:	00c05f63          	blez	a2,26c <memmove+0x28>
 252:	1602                	slli	a2,a2,0x20
 254:	9201                	srli	a2,a2,0x20
 256:	00c507b3          	add	a5,a0,a2
  dst = vdst;
 25a:	872a                	mv	a4,a0
      *dst++ = *src++;
 25c:	0585                	addi	a1,a1,1
 25e:	0705                	addi	a4,a4,1
 260:	fff5c683          	lbu	a3,-1(a1)
 264:	fed70fa3          	sb	a3,-1(a4)
    while(n-- > 0)
 268:	fef71ae3          	bne	a4,a5,25c <memmove+0x18>
    src += n;
    while(n-- > 0)
      *--dst = *--src;
  }
  return vdst;
}
 26c:	6422                	ld	s0,8(sp)
 26e:	0141                	addi	sp,sp,16
 270:	8082                	ret
    dst += n;
 272:	00c50733          	add	a4,a0,a2
    src += n;
 276:	95b2                	add	a1,a1,a2
    while(n-- > 0)
 278:	fec05ae3          	blez	a2,26c <memmove+0x28>
 27c:	fff6079b          	addiw	a5,a2,-1
 280:	1782                	slli	a5,a5,0x20
 282:	9381                	srli	a5,a5,0x20
 284:	fff7c793          	not	a5,a5
 288:	97ba                	add	a5,a5,a4
      *--dst = *--src;
 28a:	15fd                	addi	a1,a1,-1
 28c:	177d                	addi	a4,a4,-1
 28e:	0005c683          	lbu	a3,0(a1)
 292:	00d70023          	sb	a3,0(a4)
    while(n-- > 0)
 296:	fee79ae3          	bne	a5,a4,28a <memmove+0x46>
 29a:	bfc9                	j	26c <memmove+0x28>

000000000000029c <memcmp>:

int
memcmp(const void *s1, const void *s2, uint n)
{
 29c:	1141                	addi	sp,sp,-16
 29e:	e422                	sd	s0,8(sp)
 2a0:	0800                	addi	s0,sp,16
  const char *p1 = s1, *p2 = s2;
  while (n-- > 0) {
 2a2:	ca05                	beqz	a2,2d2 <memcmp+0x36>
 2a4:	fff6069b          	addiw	a3,a2,-1
 2a8:	1682                	slli	a3,a3,0x20
 2aa:	9281                	srli	a3,a3,0x20
 2ac:	0685                	addi	a3,a3,1
 2ae:	96aa                	add	a3,a3,a0
    if (*p1 != *p2) {
 2b0:	00054783          	lbu	a5,0(a0)
 2b4:	0005c703          	lbu	a4,0(a1)
 2b8:	00e79863          	bne	a5,a4,2c8 <memcmp+0x2c>
      return *p1 - *p2;
    }
    p1++;
 2bc:	0505                	addi	a0,a0,1
    p2++;
 2be:	0585                	addi	a1,a1,1
  while (n-- > 0) {
 2c0:	fed518e3          	bne	a0,a3,2b0 <memcmp+0x14>
  }
  return 0;
 2c4:	4501                	li	a0,0
 2c6:	a019                	j	2cc <memcmp+0x30>
      return *p1 - *p2;
 2c8:	40e7853b          	subw	a0,a5,a4
}
 2cc:	6422                	ld	s0,8(sp)
 2ce:	0141                	addi	sp,sp,16
 2d0:	8082                	ret
  return 0;
 2d2:	4501                	li	a0,0
 2d4:	bfe5                	j	2cc <memcmp+0x30>

00000000000002d6 <memcpy>:

void *
memcpy(void *dst, const void *src, uint n)
{
 2d6:	1141                	addi	sp,sp,-16
 2d8:	e406                	sd	ra,8(sp)
 2da:	e022                	sd	s0,0(sp)
 2dc:	0800                	addi	s0,sp,16
  return memmove(dst, src, n);
 2de:	f67ff0ef          	jal	244 <memmove>
}
 2e2:	60a2                	ld	ra,8(sp)
 2e4:	6402                	ld	s0,0(sp)
 2e6:	0141                	addi	sp,sp,16
 2e8:	8082                	ret

00000000000002ea <sbrk>:

char *
sbrk(int n) {
 2ea:	1141                	addi	sp,sp,-16
 2ec:	e406                	sd	ra,8(sp)
 2ee:	e022                	sd	s0,0(sp)
 2f0:	0800                	addi	s0,sp,16
  return sys_sbrk(n, SBRK_EAGER);
 2f2:	4585                	li	a1,1
 2f4:	0b2000ef          	jal	3a6 <sys_sbrk>
}
 2f8:	60a2                	ld	ra,8(sp)
 2fa:	6402                	ld	s0,0(sp)
 2fc:	0141                	addi	sp,sp,16
 2fe:	8082                	ret

0000000000000300 <sbrklazy>:

char *
sbrklazy(int n) {
 300:	1141                	addi	sp,sp,-16
 302:	e406                	sd	ra,8(sp)
 304:	e022                	sd	s0,0(sp)
 306:	0800                	addi	s0,sp,16
  return sys_sbrk(n, SBRK_LAZY);
 308:	4589                	li	a1,2
 30a:	09c000ef          	jal	3a6 <sys_sbrk>
}
 30e:	60a2                	ld	ra,8(sp)
 310:	6402                	ld	s0,0(sp)
 312:	0141                	addi	sp,sp,16
 314:	8082                	ret

0000000000000316 <fork>:
# generated by usys.pl - do not edit
#include "kernel/syscall.h"
.global fork
fork:
 li a7, SYS_fork
 316:	4885                	li	a7,1
 ecall
 318:	00000073          	ecall
 ret
 31c:	8082                	ret

000000000000031e <exit>:
.global exit
exit:
 li a7, SYS_exit
 31e:	4889                	li	a7,2
 ecall
 320:	00000073          	ecall
 ret
 324:	8082                	ret

0000000000000326 <wait>:
.global wait
wait:
 li a7, SYS_wait
 326:	488d                	li	a7,3
 ecall
 328:	00000073          	ecall
 ret
 32c:	8082                	ret

000000000000032e <pipe>:
.global pipe
pipe:
 li a7, SYS_pipe
 32e:	4891                	li	a7,4
 ecall
 330:	00000073          	ecall
 ret
 334:	8082                	ret

0000000000000336 <read>:
.global read
read:
 li a7, SYS_read
 336:	4895                	li	a7,5
 ecall
 338:	00000073          	ecall
 ret
 33c:	8082                	ret

000000000000033e <write>:
.global write
write:
 li a7, SYS_write
 33e:	48c1                	li	a7,16
 ecall
 340:	00000073          	ecall
 ret
 344:	8082                	ret

0000000000000346 <close>:
.global close
close:
 li a7, SYS_close
 346:	48d5                	li	a7,21
 ecall
 348:	00000073          	ecall
 ret
 34c:	8082                	ret

000000000000034e <kill>:
.global kill
kill:
 li a7, SYS_kill
 34e:	4899                	li	a7,6
 ecall
 350:	00000073          	ecall
 ret
 354:	8082                	ret

0000000000000356 <exec>:
.global exec
exec:
 li a7, SYS_exec
 356:	489d                	li	a7,7
 ecall
 358:	00000073          	ecall
 ret
 35c:	8082                	ret

000000000000035e <open>:
.global open
open:
 li a7, SYS_open
 35e:	48bd                	li	a7,15
 ecall
 360:	00000073          	ecall
 ret
 364:	8082                	ret

0000000000000366 <mknod>:
.global mknod
mknod:
 li a7, SYS_mknod
 366:	48c5                	li	a7,17
 ecall
 368:	00000073          	ecall
 ret
 36c:	8082                	ret

000000000000036e <unlink>:
.global unlink
unlink:
 li a7, SYS_unlink
 36e:	48c9                	li	a7,18
 ecall
 370:	00000073          	ecall
 ret
 374:	8082                	ret

0000000000000376 <fstat>:
.global fstat
fstat:
 li a7, SYS_fstat
 376:	48a1                	li	a7,8
 ecall
 378:	00000073          	ecall
 ret
 37c:	8082                	ret

000000000000037e <link>:
.global link
link:
 li a7, SYS_link
 37e:	48cd                	li	a7,19
 ecall
 380:	00000073          	ecall
 ret
 384:	8082                	ret

0000000000000386 <mkdir>:
.global mkdir
mkdir:
 li a7, SYS_mkdir
 386:	48d1                	li	a7,20
 ecall
 388:	00000073          	ecall
 ret
 38c:	8082                	ret

000000000000038e <chdir>:
.global chdir
chdir:
 li a7, SYS_chdir
 38e:	48a5                	li	a7,9
 ecall
 390:	00000073          	ecall
 ret
 394:	8082                	ret

0000000000000396 <dup>:
.global dup
dup:
 li a7, SYS_dup
 396:	48a9                	li	a7,10
 ecall
 398:	00000073          	ecall
 ret
 39c:	8082                	ret

000000000000039e <getpid>:
.global getpid
getpid:
 li a7, SYS_getpid
 39e:	48ad                	li	a7,11
 ecall
 3a0:	00000073          	ecall
 ret
 3a4:	8082                	ret

00000000000003a6 <sys_sbrk>:
.global sys_sbrk
sys_sbrk:
 li a7, SYS_sbrk
 3a6:	48b1                	li	a7,12
 ecall
 3a8:	00000073          	ecall
 ret
 3ac:	8082                	ret

00000000000003ae <pause>:
.global pause
pause:
 li a7, SYS_pause
 3ae:	48b5                	li	a7,13
 ecall
 3b0:	00000073          	ecall
 ret
 3b4:	8082                	ret

00000000000003b6 <uptime>:
.global uptime
uptime:
 li a7, SYS_uptime
 3b6:	48b9                	li	a7,14
 ecall
 3b8:	00000073          	ecall
 ret
 3bc:	8082                	ret

00000000000003be <putc>:

static char digits[] = "0123456789ABCDEF";

static void
putc(int fd, char c)
{
 3be:	1101                	addi	sp,sp,-32
 3c0:	ec06                	sd	ra,24(sp)
 3c2:	e822                	sd	s0,16(sp)
 3c4:	1000                	addi	s0,sp,32
 3c6:	feb407a3          	sb	a1,-17(s0)
  write(fd, &c, 1);
 3ca:	4605                	li	a2,1
 3cc:	fef40593          	addi	a1,s0,-17
 3d0:	f6fff0ef          	jal	33e <write>
}
 3d4:	60e2                	ld	ra,24(sp)
 3d6:	6442                	ld	s0,16(sp)
 3d8:	6105                	addi	sp,sp,32
 3da:	8082                	ret

00000000000003dc <printint>:

static void
printint(int fd, long long xx, int base, int sgn)
{
 3dc:	715d                	addi	sp,sp,-80
 3de:	e486                	sd	ra,72(sp)
 3e0:	e0a2                	sd	s0,64(sp)
 3e2:	fc26                	sd	s1,56(sp)
 3e4:	0880                	addi	s0,sp,80
 3e6:	84aa                	mv	s1,a0
  char buf[20];
  int i, neg;
  uint x;

  neg = 0;
  if(sgn && xx < 0){
 3e8:	c299                	beqz	a3,3ee <printint+0x12>
 3ea:	0805c963          	bltz	a1,47c <printint+0xa0>
    neg = 1;
    x = -xx;
  } else {
    x = xx;
 3ee:	2581                	sext.w	a1,a1
  neg = 0;
 3f0:	4881                	li	a7,0
 3f2:	fb840693          	addi	a3,s0,-72
  }

  i = 0;
 3f6:	4701                	li	a4,0
  do{
    buf[i++] = digits[x % base];
 3f8:	2601                	sext.w	a2,a2
 3fa:	00000517          	auipc	a0,0x0
 3fe:	58650513          	addi	a0,a0,1414 # 980 <digits>
 402:	883a                	mv	a6,a4
 404:	2705                	addiw	a4,a4,1
 406:	02c5f7bb          	remuw	a5,a1,a2
 40a:	1782                	slli	a5,a5,0x20
 40c:	9381                	srli	a5,a5,0x20
 40e:	97aa                	add	a5,a5,a0
 410:	0007c783          	lbu	a5,0(a5)
 414:	00f68023          	sb	a5,0(a3)
  }while((x /= base) != 0);
 418:	0005879b          	sext.w	a5,a1
 41c:	02c5d5bb          	divuw	a1,a1,a2
 420:	0685                	addi	a3,a3,1
 422:	fec7f0e3          	bgeu	a5,a2,402 <printint+0x26>
  if(neg)
 426:	00088c63          	beqz	a7,43e <printint+0x62>
    buf[i++] = '-';
 42a:	fd070793          	addi	a5,a4,-48
 42e:	00878733          	add	a4,a5,s0
 432:	02d00793          	li	a5,45
 436:	fef70423          	sb	a5,-24(a4)
 43a:	0028071b          	addiw	a4,a6,2

  while(--i >= 0)
 43e:	02e05a63          	blez	a4,472 <printint+0x96>
 442:	f84a                	sd	s2,48(sp)
 444:	f44e                	sd	s3,40(sp)
 446:	fb840793          	addi	a5,s0,-72
 44a:	00e78933          	add	s2,a5,a4
 44e:	fff78993          	addi	s3,a5,-1
 452:	99ba                	add	s3,s3,a4
 454:	377d                	addiw	a4,a4,-1
 456:	1702                	slli	a4,a4,0x20
 458:	9301                	srli	a4,a4,0x20
 45a:	40e989b3          	sub	s3,s3,a4
    putc(fd, buf[i]);
 45e:	fff94583          	lbu	a1,-1(s2)
 462:	8526                	mv	a0,s1
 464:	f5bff0ef          	jal	3be <putc>
  while(--i >= 0)
 468:	197d                	addi	s2,s2,-1
 46a:	ff391ae3          	bne	s2,s3,45e <printint+0x82>
 46e:	7942                	ld	s2,48(sp)
 470:	79a2                	ld	s3,40(sp)
}
 472:	60a6                	ld	ra,72(sp)
 474:	6406                	ld	s0,64(sp)
 476:	74e2                	ld	s1,56(sp)
 478:	6161                	addi	sp,sp,80
 47a:	8082                	ret
    x = -xx;
 47c:	40b005bb          	negw	a1,a1
    neg = 1;
 480:	4885                	li	a7,1
    x = -xx;
 482:	bf85                	j	3f2 <printint+0x16>

0000000000000484 <vprintf>:
}

// Print to the given fd. Only understands %d, %x, %p, %c, %s.
void
vprintf(int fd, const char *fmt, va_list ap)
{
 484:	711d                	addi	sp,sp,-96
 486:	ec86                	sd	ra,88(sp)
 488:	e8a2                	sd	s0,80(sp)
 48a:	e0ca                	sd	s2,64(sp)
 48c:	1080                	addi	s0,sp,96
  char *s;
  int c0, c1, c2, i, state;

  state = 0;
  for(i = 0; fmt[i]; i++){
 48e:	0005c903          	lbu	s2,0(a1)
 492:	28090663          	beqz	s2,71e <vprintf+0x29a>
 496:	e4a6                	sd	s1,72(sp)
 498:	fc4e                	sd	s3,56(sp)
 49a:	f852                	sd	s4,48(sp)
 49c:	f456                	sd	s5,40(sp)
 49e:	f05a                	sd	s6,32(sp)
 4a0:	ec5e                	sd	s7,24(sp)
 4a2:	e862                	sd	s8,16(sp)
 4a4:	e466                	sd	s9,8(sp)
 4a6:	8b2a                	mv	s6,a0
 4a8:	8a2e                	mv	s4,a1
 4aa:	8bb2                	mv	s7,a2
  state = 0;
 4ac:	4981                	li	s3,0
  for(i = 0; fmt[i]; i++){
 4ae:	4481                	li	s1,0
 4b0:	4701                	li	a4,0
      if(c0 == '%'){
        state = '%';
      } else {
        putc(fd, c0);
      }
    } else if(state == '%'){
 4b2:	02500a93          	li	s5,37
      c1 = c2 = 0;
      if(c0) c1 = fmt[i+1] & 0xff;
      if(c1) c2 = fmt[i+2] & 0xff;
      if(c0 == 'd'){
 4b6:	06400c13          	li	s8,100
        printint(fd, va_arg(ap, int), 10, 1);
      } else if(c0 == 'l' && c1 == 'd'){
 4ba:	06c00c93          	li	s9,108
 4be:	a005                	j	4de <vprintf+0x5a>
        putc(fd, c0);
 4c0:	85ca                	mv	a1,s2
 4c2:	855a                	mv	a0,s6
 4c4:	efbff0ef          	jal	3be <putc>
 4c8:	a019                	j	4ce <vprintf+0x4a>
    } else if(state == '%'){
 4ca:	03598263          	beq	s3,s5,4ee <vprintf+0x6a>
  for(i = 0; fmt[i]; i++){
 4ce:	2485                	addiw	s1,s1,1
 4d0:	8726                	mv	a4,s1
 4d2:	009a07b3          	add	a5,s4,s1
 4d6:	0007c903          	lbu	s2,0(a5)
 4da:	22090a63          	beqz	s2,70e <vprintf+0x28a>
    c0 = fmt[i] & 0xff;
 4de:	0009079b          	sext.w	a5,s2
    if(state == 0){
 4e2:	fe0994e3          	bnez	s3,4ca <vprintf+0x46>
      if(c0 == '%'){
 4e6:	fd579de3          	bne	a5,s5,4c0 <vprintf+0x3c>
        state = '%';
 4ea:	89be                	mv	s3,a5
 4ec:	b7cd                	j	4ce <vprintf+0x4a>
      if(c0) c1 = fmt[i+1] & 0xff;
 4ee:	00ea06b3          	add	a3,s4,a4
 4f2:	0016c683          	lbu	a3,1(a3)
      c1 = c2 = 0;
 4f6:	8636                	mv	a2,a3
      if(c1) c2 = fmt[i+2] & 0xff;
 4f8:	c681                	beqz	a3,500 <vprintf+0x7c>
 4fa:	9752                	add	a4,a4,s4
 4fc:	00274603          	lbu	a2,2(a4)
      if(c0 == 'd'){
 500:	05878363          	beq	a5,s8,546 <vprintf+0xc2>
      } else if(c0 == 'l' && c1 == 'd'){
 504:	05978d63          	beq	a5,s9,55e <vprintf+0xda>
        printint(fd, va_arg(ap, uint64), 10, 1);
        i += 1;
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'd'){
        printint(fd, va_arg(ap, uint64), 10, 1);
        i += 2;
      } else if(c0 == 'u'){
 508:	07500713          	li	a4,117
 50c:	0ee78763          	beq	a5,a4,5fa <vprintf+0x176>
        printint(fd, va_arg(ap, uint64), 10, 0);
        i += 1;
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'u'){
        printint(fd, va_arg(ap, uint64), 10, 0);
        i += 2;
      } else if(c0 == 'x'){
 510:	07800713          	li	a4,120
 514:	12e78963          	beq	a5,a4,646 <vprintf+0x1c2>
        printint(fd, va_arg(ap, uint64), 16, 0);
        i += 1;
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'x'){
        printint(fd, va_arg(ap, uint64), 16, 0);
        i += 2;
      } else if(c0 == 'p'){
 518:	07000713          	li	a4,112
 51c:	14e78e63          	beq	a5,a4,678 <vprintf+0x1f4>
        printptr(fd, va_arg(ap, uint64));
      } else if(c0 == 'c'){
 520:	06300713          	li	a4,99
 524:	18e78e63          	beq	a5,a4,6c0 <vprintf+0x23c>
        putc(fd, va_arg(ap, uint32));
      } else if(c0 == 's'){
 528:	07300713          	li	a4,115
 52c:	1ae78463          	beq	a5,a4,6d4 <vprintf+0x250>
        if((s = va_arg(ap, char*)) == 0)
          s = "(null)";
        for(; *s; s++)
          putc(fd, *s);
      } else if(c0 == '%'){
 530:	02500713          	li	a4,37
 534:	04e79563          	bne	a5,a4,57e <vprintf+0xfa>
        putc(fd, '%');
 538:	02500593          	li	a1,37
 53c:	855a                	mv	a0,s6
 53e:	e81ff0ef          	jal	3be <putc>
        // Unknown % sequence.  Print it to draw attention.
        putc(fd, '%');
        putc(fd, c0);
      }

      state = 0;
 542:	4981                	li	s3,0
 544:	b769                	j	4ce <vprintf+0x4a>
        printint(fd, va_arg(ap, int), 10, 1);
 546:	008b8913          	addi	s2,s7,8
 54a:	4685                	li	a3,1
 54c:	4629                	li	a2,10
 54e:	000ba583          	lw	a1,0(s7)
 552:	855a                	mv	a0,s6
 554:	e89ff0ef          	jal	3dc <printint>
 558:	8bca                	mv	s7,s2
      state = 0;
 55a:	4981                	li	s3,0
 55c:	bf8d                	j	4ce <vprintf+0x4a>
      } else if(c0 == 'l' && c1 == 'd'){
 55e:	06400793          	li	a5,100
 562:	02f68963          	beq	a3,a5,594 <vprintf+0x110>
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'd'){
 566:	06c00793          	li	a5,108
 56a:	04f68263          	beq	a3,a5,5ae <vprintf+0x12a>
      } else if(c0 == 'l' && c1 == 'u'){
 56e:	07500793          	li	a5,117
 572:	0af68063          	beq	a3,a5,612 <vprintf+0x18e>
      } else if(c0 == 'l' && c1 == 'x'){
 576:	07800793          	li	a5,120
 57a:	0ef68263          	beq	a3,a5,65e <vprintf+0x1da>
        putc(fd, '%');
 57e:	02500593          	li	a1,37
 582:	855a                	mv	a0,s6
 584:	e3bff0ef          	jal	3be <putc>
        putc(fd, c0);
 588:	85ca                	mv	a1,s2
 58a:	855a                	mv	a0,s6
 58c:	e33ff0ef          	jal	3be <putc>
      state = 0;
 590:	4981                	li	s3,0
 592:	bf35                	j	4ce <vprintf+0x4a>
        printint(fd, va_arg(ap, uint64), 10, 1);
 594:	008b8913          	addi	s2,s7,8
 598:	4685                	li	a3,1
 59a:	4629                	li	a2,10
 59c:	000bb583          	ld	a1,0(s7)
 5a0:	855a                	mv	a0,s6
 5a2:	e3bff0ef          	jal	3dc <printint>
        i += 1;
 5a6:	2485                	addiw	s1,s1,1
        printint(fd, va_arg(ap, uint64), 10, 1);
 5a8:	8bca                	mv	s7,s2
      state = 0;
 5aa:	4981                	li	s3,0
        i += 1;
 5ac:	b70d                	j	4ce <vprintf+0x4a>
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'd'){
 5ae:	06400793          	li	a5,100
 5b2:	02f60763          	beq	a2,a5,5e0 <vprintf+0x15c>
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'u'){
 5b6:	07500793          	li	a5,117
 5ba:	06f60963          	beq	a2,a5,62c <vprintf+0x1a8>
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'x'){
 5be:	07800793          	li	a5,120
 5c2:	faf61ee3          	bne	a2,a5,57e <vprintf+0xfa>
        printint(fd, va_arg(ap, uint64), 16, 0);
 5c6:	008b8913          	addi	s2,s7,8
 5ca:	4681                	li	a3,0
 5cc:	4641                	li	a2,16
 5ce:	000bb583          	ld	a1,0(s7)
 5d2:	855a                	mv	a0,s6
 5d4:	e09ff0ef          	jal	3dc <printint>
        i += 2;
 5d8:	2489                	addiw	s1,s1,2
        printint(fd, va_arg(ap, uint64), 16, 0);
 5da:	8bca                	mv	s7,s2
      state = 0;
 5dc:	4981                	li	s3,0
        i += 2;
 5de:	bdc5                	j	4ce <vprintf+0x4a>
        printint(fd, va_arg(ap, uint64), 10, 1);
 5e0:	008b8913          	addi	s2,s7,8
 5e4:	4685                	li	a3,1
 5e6:	4629                	li	a2,10
 5e8:	000bb583          	ld	a1,0(s7)
 5ec:	855a                	mv	a0,s6
 5ee:	defff0ef          	jal	3dc <printint>
        i += 2;
 5f2:	2489                	addiw	s1,s1,2
        printint(fd, va_arg(ap, uint64), 10, 1);
 5f4:	8bca                	mv	s7,s2
      state = 0;
 5f6:	4981                	li	s3,0
        i += 2;
 5f8:	bdd9                	j	4ce <vprintf+0x4a>
        printint(fd, va_arg(ap, uint32), 10, 0);
 5fa:	008b8913          	addi	s2,s7,8
 5fe:	4681                	li	a3,0
 600:	4629                	li	a2,10
 602:	000be583          	lwu	a1,0(s7)
 606:	855a                	mv	a0,s6
 608:	dd5ff0ef          	jal	3dc <printint>
 60c:	8bca                	mv	s7,s2
      state = 0;
 60e:	4981                	li	s3,0
 610:	bd7d                	j	4ce <vprintf+0x4a>
        printint(fd, va_arg(ap, uint64), 10, 0);
 612:	008b8913          	addi	s2,s7,8
 616:	4681                	li	a3,0
 618:	4629                	li	a2,10
 61a:	000bb583          	ld	a1,0(s7)
 61e:	855a                	mv	a0,s6
 620:	dbdff0ef          	jal	3dc <printint>
        i += 1;
 624:	2485                	addiw	s1,s1,1
        printint(fd, va_arg(ap, uint64), 10, 0);
 626:	8bca                	mv	s7,s2
      state = 0;
 628:	4981                	li	s3,0
        i += 1;
 62a:	b555                	j	4ce <vprintf+0x4a>
        printint(fd, va_arg(ap, uint64), 10, 0);
 62c:	008b8913          	addi	s2,s7,8
 630:	4681                	li	a3,0
 632:	4629                	li	a2,10
 634:	000bb583          	ld	a1,0(s7)
 638:	855a                	mv	a0,s6
 63a:	da3ff0ef          	jal	3dc <printint>
        i += 2;
 63e:	2489                	addiw	s1,s1,2
        printint(fd, va_arg(ap, uint64), 10, 0);
 640:	8bca                	mv	s7,s2
      state = 0;
 642:	4981                	li	s3,0
        i += 2;
 644:	b569                	j	4ce <vprintf+0x4a>
        printint(fd, va_arg(ap, uint32), 16, 0);
 646:	008b8913          	addi	s2,s7,8
 64a:	4681                	li	a3,0
 64c:	4641                	li	a2,16
 64e:	000be583          	lwu	a1,0(s7)
 652:	855a                	mv	a0,s6
 654:	d89ff0ef          	jal	3dc <printint>
 658:	8bca                	mv	s7,s2
      state = 0;
 65a:	4981                	li	s3,0
 65c:	bd8d                	j	4ce <vprintf+0x4a>
        printint(fd, va_arg(ap, uint64), 16, 0);
 65e:	008b8913          	addi	s2,s7,8
 662:	4681                	li	a3,0
 664:	4641                	li	a2,16
 666:	000bb583          	ld	a1,0(s7)
 66a:	855a                	mv	a0,s6
 66c:	d71ff0ef          	jal	3dc <printint>
        i += 1;
 670:	2485                	addiw	s1,s1,1
        printint(fd, va_arg(ap, uint64), 16, 0);
 672:	8bca                	mv	s7,s2
      state = 0;
 674:	4981                	li	s3,0
        i += 1;
 676:	bda1                	j	4ce <vprintf+0x4a>
 678:	e06a                	sd	s10,0(sp)
        printptr(fd, va_arg(ap, uint64));
 67a:	008b8d13          	addi	s10,s7,8
 67e:	000bb983          	ld	s3,0(s7)
  putc(fd, '0');
 682:	03000593          	li	a1,48
 686:	855a                	mv	a0,s6
 688:	d37ff0ef          	jal	3be <putc>
  putc(fd, 'x');
 68c:	07800593          	li	a1,120
 690:	855a                	mv	a0,s6
 692:	d2dff0ef          	jal	3be <putc>
 696:	4941                	li	s2,16
    putc(fd, digits[x >> (sizeof(uint64) * 8 - 4)]);
 698:	00000b97          	auipc	s7,0x0
 69c:	2e8b8b93          	addi	s7,s7,744 # 980 <digits>
 6a0:	03c9d793          	srli	a5,s3,0x3c
 6a4:	97de                	add	a5,a5,s7
 6a6:	0007c583          	lbu	a1,0(a5)
 6aa:	855a                	mv	a0,s6
 6ac:	d13ff0ef          	jal	3be <putc>
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
 6b0:	0992                	slli	s3,s3,0x4
 6b2:	397d                	addiw	s2,s2,-1
 6b4:	fe0916e3          	bnez	s2,6a0 <vprintf+0x21c>
        printptr(fd, va_arg(ap, uint64));
 6b8:	8bea                	mv	s7,s10
      state = 0;
 6ba:	4981                	li	s3,0
 6bc:	6d02                	ld	s10,0(sp)
 6be:	bd01                	j	4ce <vprintf+0x4a>
        putc(fd, va_arg(ap, uint32));
 6c0:	008b8913          	addi	s2,s7,8
 6c4:	000bc583          	lbu	a1,0(s7)
 6c8:	855a                	mv	a0,s6
 6ca:	cf5ff0ef          	jal	3be <putc>
 6ce:	8bca                	mv	s7,s2
      state = 0;
 6d0:	4981                	li	s3,0
 6d2:	bbf5                	j	4ce <vprintf+0x4a>
        if((s = va_arg(ap, char*)) == 0)
 6d4:	008b8993          	addi	s3,s7,8
 6d8:	000bb903          	ld	s2,0(s7)
 6dc:	00090f63          	beqz	s2,6fa <vprintf+0x276>
        for(; *s; s++)
 6e0:	00094583          	lbu	a1,0(s2)
 6e4:	c195                	beqz	a1,708 <vprintf+0x284>
          putc(fd, *s);
 6e6:	855a                	mv	a0,s6
 6e8:	cd7ff0ef          	jal	3be <putc>
        for(; *s; s++)
 6ec:	0905                	addi	s2,s2,1
 6ee:	00094583          	lbu	a1,0(s2)
 6f2:	f9f5                	bnez	a1,6e6 <vprintf+0x262>
        if((s = va_arg(ap, char*)) == 0)
 6f4:	8bce                	mv	s7,s3
      state = 0;
 6f6:	4981                	li	s3,0
 6f8:	bbd9                	j	4ce <vprintf+0x4a>
          s = "(null)";
 6fa:	00000917          	auipc	s2,0x0
 6fe:	27e90913          	addi	s2,s2,638 # 978 <malloc+0x172>
        for(; *s; s++)
 702:	02800593          	li	a1,40
 706:	b7c5                	j	6e6 <vprintf+0x262>
        if((s = va_arg(ap, char*)) == 0)
 708:	8bce                	mv	s7,s3
      state = 0;
 70a:	4981                	li	s3,0
 70c:	b3c9                	j	4ce <vprintf+0x4a>
 70e:	64a6                	ld	s1,72(sp)
 710:	79e2                	ld	s3,56(sp)
 712:	7a42                	ld	s4,48(sp)
 714:	7aa2                	ld	s5,40(sp)
 716:	7b02                	ld	s6,32(sp)
 718:	6be2                	ld	s7,24(sp)
 71a:	6c42                	ld	s8,16(sp)
 71c:	6ca2                	ld	s9,8(sp)
    }
  }
}
 71e:	60e6                	ld	ra,88(sp)
 720:	6446                	ld	s0,80(sp)
 722:	6906                	ld	s2,64(sp)
 724:	6125                	addi	sp,sp,96
 726:	8082                	ret

0000000000000728 <fprintf>:

void
fprintf(int fd, const char *fmt, ...)
{
 728:	715d                	addi	sp,sp,-80
 72a:	ec06                	sd	ra,24(sp)
 72c:	e822                	sd	s0,16(sp)
 72e:	1000                	addi	s0,sp,32
 730:	e010                	sd	a2,0(s0)
 732:	e414                	sd	a3,8(s0)
 734:	e818                	sd	a4,16(s0)
 736:	ec1c                	sd	a5,24(s0)
 738:	03043023          	sd	a6,32(s0)
 73c:	03143423          	sd	a7,40(s0)
  va_list ap;

  va_start(ap, fmt);
 740:	fe843423          	sd	s0,-24(s0)
  vprintf(fd, fmt, ap);
 744:	8622                	mv	a2,s0
 746:	d3fff0ef          	jal	484 <vprintf>
}
 74a:	60e2                	ld	ra,24(sp)
 74c:	6442                	ld	s0,16(sp)
 74e:	6161                	addi	sp,sp,80
 750:	8082                	ret

0000000000000752 <printf>:

void
printf(const char *fmt, ...)
{
 752:	711d                	addi	sp,sp,-96
 754:	ec06                	sd	ra,24(sp)
 756:	e822                	sd	s0,16(sp)
 758:	1000                	addi	s0,sp,32
 75a:	e40c                	sd	a1,8(s0)
 75c:	e810                	sd	a2,16(s0)
 75e:	ec14                	sd	a3,24(s0)
 760:	f018                	sd	a4,32(s0)
 762:	f41c                	sd	a5,40(s0)
 764:	03043823          	sd	a6,48(s0)
 768:	03143c23          	sd	a7,56(s0)
  va_list ap;

  va_start(ap, fmt);
 76c:	00840613          	addi	a2,s0,8
 770:	fec43423          	sd	a2,-24(s0)
  vprintf(1, fmt, ap);
 774:	85aa                	mv	a1,a0
 776:	4505                	li	a0,1
 778:	d0dff0ef          	jal	484 <vprintf>
}
 77c:	60e2                	ld	ra,24(sp)
 77e:	6442                	ld	s0,16(sp)
 780:	6125                	addi	sp,sp,96
 782:	8082                	ret

0000000000000784 <free>:
static Header base;
static Header *freep;

void
free(void *ap)
{
 784:	1141                	addi	sp,sp,-16
 786:	e422                	sd	s0,8(sp)
 788:	0800                	addi	s0,sp,16
  Header *bp, *p;

  bp = (Header*)ap - 1;
 78a:	ff050693          	addi	a3,a0,-16
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
 78e:	00001797          	auipc	a5,0x1
 792:	8727b783          	ld	a5,-1934(a5) # 1000 <freep>
 796:	a02d                	j	7c0 <free+0x3c>
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;
  if(bp + bp->s.size == p->s.ptr){
    bp->s.size += p->s.ptr->s.size;
 798:	4618                	lw	a4,8(a2)
 79a:	9f2d                	addw	a4,a4,a1
 79c:	fee52c23          	sw	a4,-8(a0)
    bp->s.ptr = p->s.ptr->s.ptr;
 7a0:	6398                	ld	a4,0(a5)
 7a2:	6310                	ld	a2,0(a4)
 7a4:	a83d                	j	7e2 <free+0x5e>
  } else
    bp->s.ptr = p->s.ptr;
  if(p + p->s.size == bp){
    p->s.size += bp->s.size;
 7a6:	ff852703          	lw	a4,-8(a0)
 7aa:	9f31                	addw	a4,a4,a2
 7ac:	c798                	sw	a4,8(a5)
    p->s.ptr = bp->s.ptr;
 7ae:	ff053683          	ld	a3,-16(a0)
 7b2:	a091                	j	7f6 <free+0x72>
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
 7b4:	6398                	ld	a4,0(a5)
 7b6:	00e7e463          	bltu	a5,a4,7be <free+0x3a>
 7ba:	00e6ea63          	bltu	a3,a4,7ce <free+0x4a>
{
 7be:	87ba                	mv	a5,a4
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
 7c0:	fed7fae3          	bgeu	a5,a3,7b4 <free+0x30>
 7c4:	6398                	ld	a4,0(a5)
 7c6:	00e6e463          	bltu	a3,a4,7ce <free+0x4a>
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
 7ca:	fee7eae3          	bltu	a5,a4,7be <free+0x3a>
  if(bp + bp->s.size == p->s.ptr){
 7ce:	ff852583          	lw	a1,-8(a0)
 7d2:	6390                	ld	a2,0(a5)
 7d4:	02059813          	slli	a6,a1,0x20
 7d8:	01c85713          	srli	a4,a6,0x1c
 7dc:	9736                	add	a4,a4,a3
 7de:	fae60de3          	beq	a2,a4,798 <free+0x14>
    bp->s.ptr = p->s.ptr->s.ptr;
 7e2:	fec53823          	sd	a2,-16(a0)
  if(p + p->s.size == bp){
 7e6:	4790                	lw	a2,8(a5)
 7e8:	02061593          	slli	a1,a2,0x20
 7ec:	01c5d713          	srli	a4,a1,0x1c
 7f0:	973e                	add	a4,a4,a5
 7f2:	fae68ae3          	beq	a3,a4,7a6 <free+0x22>
    p->s.ptr = bp->s.ptr;
 7f6:	e394                	sd	a3,0(a5)
  } else
    p->s.ptr = bp;
  freep = p;
 7f8:	00001717          	auipc	a4,0x1
 7fc:	80f73423          	sd	a5,-2040(a4) # 1000 <freep>
}
 800:	6422                	ld	s0,8(sp)
 802:	0141                	addi	sp,sp,16
 804:	8082                	ret

0000000000000806 <malloc>:
  return freep;
}

void*
malloc(uint nbytes)
{
 806:	7139                	addi	sp,sp,-64
 808:	fc06                	sd	ra,56(sp)
 80a:	f822                	sd	s0,48(sp)
 80c:	f426                	sd	s1,40(sp)
 80e:	ec4e                	sd	s3,24(sp)
 810:	0080                	addi	s0,sp,64
  Header *p, *prevp;
  uint nunits;

  nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
 812:	02051493          	slli	s1,a0,0x20
 816:	9081                	srli	s1,s1,0x20
 818:	04bd                	addi	s1,s1,15
 81a:	8091                	srli	s1,s1,0x4
 81c:	0014899b          	addiw	s3,s1,1
 820:	0485                	addi	s1,s1,1
  if((prevp = freep) == 0){
 822:	00000517          	auipc	a0,0x0
 826:	7de53503          	ld	a0,2014(a0) # 1000 <freep>
 82a:	c915                	beqz	a0,85e <malloc+0x58>
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
 82c:	611c                	ld	a5,0(a0)
    if(p->s.size >= nunits){
 82e:	4798                	lw	a4,8(a5)
 830:	08977a63          	bgeu	a4,s1,8c4 <malloc+0xbe>
 834:	f04a                	sd	s2,32(sp)
 836:	e852                	sd	s4,16(sp)
 838:	e456                	sd	s5,8(sp)
 83a:	e05a                	sd	s6,0(sp)
  if(nu < 4096)
 83c:	8a4e                	mv	s4,s3
 83e:	0009871b          	sext.w	a4,s3
 842:	6685                	lui	a3,0x1
 844:	00d77363          	bgeu	a4,a3,84a <malloc+0x44>
 848:	6a05                	lui	s4,0x1
 84a:	000a0b1b          	sext.w	s6,s4
  p = sbrk(nu * sizeof(Header));
 84e:	004a1a1b          	slliw	s4,s4,0x4
        p->s.size = nunits;
      }
      freep = prevp;
      return (void*)(p + 1);
    }
    if(p == freep)
 852:	00000917          	auipc	s2,0x0
 856:	7ae90913          	addi	s2,s2,1966 # 1000 <freep>
  if(p == SBRK_ERROR)
 85a:	5afd                	li	s5,-1
 85c:	a081                	j	89c <malloc+0x96>
 85e:	f04a                	sd	s2,32(sp)
 860:	e852                	sd	s4,16(sp)
 862:	e456                	sd	s5,8(sp)
 864:	e05a                	sd	s6,0(sp)
    base.s.ptr = freep = prevp = &base;
 866:	00001797          	auipc	a5,0x1
 86a:	9a278793          	addi	a5,a5,-1630 # 1208 <base>
 86e:	00000717          	auipc	a4,0x0
 872:	78f73923          	sd	a5,1938(a4) # 1000 <freep>
 876:	e39c                	sd	a5,0(a5)
    base.s.size = 0;
 878:	0007a423          	sw	zero,8(a5)
    if(p->s.size >= nunits){
 87c:	b7c1                	j	83c <malloc+0x36>
        prevp->s.ptr = p->s.ptr;
 87e:	6398                	ld	a4,0(a5)
 880:	e118                	sd	a4,0(a0)
 882:	a8a9                	j	8dc <malloc+0xd6>
  hp->s.size = nu;
 884:	01652423          	sw	s6,8(a0)
  free((void*)(hp + 1));
 888:	0541                	addi	a0,a0,16
 88a:	efbff0ef          	jal	784 <free>
  return freep;
 88e:	00093503          	ld	a0,0(s2)
      if((p = morecore(nunits)) == 0)
 892:	c12d                	beqz	a0,8f4 <malloc+0xee>
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
 894:	611c                	ld	a5,0(a0)
    if(p->s.size >= nunits){
 896:	4798                	lw	a4,8(a5)
 898:	02977263          	bgeu	a4,s1,8bc <malloc+0xb6>
    if(p == freep)
 89c:	00093703          	ld	a4,0(s2)
 8a0:	853e                	mv	a0,a5
 8a2:	fef719e3          	bne	a4,a5,894 <malloc+0x8e>
  p = sbrk(nu * sizeof(Header));
 8a6:	8552                	mv	a0,s4
 8a8:	a43ff0ef          	jal	2ea <sbrk>
  if(p == SBRK_ERROR)
 8ac:	fd551ce3          	bne	a0,s5,884 <malloc+0x7e>
        return 0;
 8b0:	4501                	li	a0,0
 8b2:	7902                	ld	s2,32(sp)
 8b4:	6a42                	ld	s4,16(sp)
 8b6:	6aa2                	ld	s5,8(sp)
 8b8:	6b02                	ld	s6,0(sp)
 8ba:	a03d                	j	8e8 <malloc+0xe2>
 8bc:	7902                	ld	s2,32(sp)
 8be:	6a42                	ld	s4,16(sp)
 8c0:	6aa2                	ld	s5,8(sp)
 8c2:	6b02                	ld	s6,0(sp)
      if(p->s.size == nunits)
 8c4:	fae48de3          	beq	s1,a4,87e <malloc+0x78>
        p->s.size -= nunits;
 8c8:	4137073b          	subw	a4,a4,s3
 8cc:	c798                	sw	a4,8(a5)
        p += p->s.size;
 8ce:	02071693          	slli	a3,a4,0x20
 8d2:	01c6d713          	srli	a4,a3,0x1c
 8d6:	97ba                	add	a5,a5,a4
        p->s.size = nunits;
 8d8:	0137a423          	sw	s3,8(a5)
      freep = prevp;
 8dc:	00000717          	auipc	a4,0x0
 8e0:	72a73223          	sd	a0,1828(a4) # 1000 <freep>
      return (void*)(p + 1);
 8e4:	01078513          	addi	a0,a5,16
  }
}
 8e8:	70e2                	ld	ra,56(sp)
 8ea:	7442                	ld	s0,48(sp)
 8ec:	74a2                	ld	s1,40(sp)
 8ee:	69e2                	ld	s3,24(sp)
 8f0:	6121                	addi	sp,sp,64
 8f2:	8082                	ret
 8f4:	7902                	ld	s2,32(sp)
 8f6:	6a42                	ld	s4,16(sp)
 8f8:	6aa2                	ld	s5,8(sp)
 8fa:	6b02                	ld	s6,0(sp)
 8fc:	b7f5                	j	8e8 <malloc+0xe2>
