
user/_logstress:     file format elf64-littleriscv


Disassembly of section .text:

0000000000000000 <main>:
main(int argc, char **argv)
{
  int fd, n;
  enum { N = 250, SZ=2000 };
  
  for (int i = 1; i < argc; i++){
   0:	4785                	li	a5,1
   2:	0ea7df63          	bge	a5,a0,100 <main+0x100>
{
   6:	7139                	addi	sp,sp,-64
   8:	fc06                	sd	ra,56(sp)
   a:	f822                	sd	s0,48(sp)
   c:	f426                	sd	s1,40(sp)
   e:	f04a                	sd	s2,32(sp)
  10:	ec4e                	sd	s3,24(sp)
  12:	0080                	addi	s0,sp,64
  14:	892a                	mv	s2,a0
  16:	89ae                	mv	s3,a1
  for (int i = 1; i < argc; i++){
  18:	4485                	li	s1,1
  1a:	a011                	j	1e <main+0x1e>
  1c:	84be                	mv	s1,a5
    int pid1 = fork();
  1e:	372000ef          	jal	390 <fork>
    if(pid1 < 0){
  22:	00054963          	bltz	a0,34 <main+0x34>
      printf("%s: fork failed\n", argv[0]);
      exit(1);
    }
    if(pid1 == 0) {
  26:	c11d                	beqz	a0,4c <main+0x4c>
  for (int i = 1; i < argc; i++){
  28:	0014879b          	addiw	a5,s1,1
  2c:	fef918e3          	bne	s2,a5,1c <main+0x1c>
      }
      exit(0);
    }
  }
  int xstatus;
  for(int i = 1; i < argc; i++){
  30:	4905                	li	s2,1
  32:	a04d                	j	d4 <main+0xd4>
  34:	e852                	sd	s4,16(sp)
      printf("%s: fork failed\n", argv[0]);
  36:	0009b583          	ld	a1,0(s3)
  3a:	00001517          	auipc	a0,0x1
  3e:	94650513          	addi	a0,a0,-1722 # 980 <malloc+0x100>
  42:	78a000ef          	jal	7cc <printf>
      exit(1);
  46:	4505                	li	a0,1
  48:	350000ef          	jal	398 <exit>
  4c:	e852                	sd	s4,16(sp)
      fd = open(argv[i], O_CREATE | O_RDWR);
  4e:	00349a13          	slli	s4,s1,0x3
  52:	9a4e                	add	s4,s4,s3
  54:	20200593          	li	a1,514
  58:	000a3503          	ld	a0,0(s4)
  5c:	37c000ef          	jal	3d8 <open>
  60:	892a                	mv	s2,a0
      if(fd < 0){
  62:	04054163          	bltz	a0,a4 <main+0xa4>
      memset(buf, '0'+i, SZ);
  66:	7d000613          	li	a2,2000
  6a:	0304859b          	addiw	a1,s1,48
  6e:	00001517          	auipc	a0,0x1
  72:	fa250513          	addi	a0,a0,-94 # 1010 <buf>
  76:	110000ef          	jal	186 <memset>
  7a:	0fa00493          	li	s1,250
        if((n = write(fd, buf, SZ)) != SZ){
  7e:	00001997          	auipc	s3,0x1
  82:	f9298993          	addi	s3,s3,-110 # 1010 <buf>
  86:	7d000613          	li	a2,2000
  8a:	85ce                	mv	a1,s3
  8c:	854a                	mv	a0,s2
  8e:	32a000ef          	jal	3b8 <write>
  92:	7d000793          	li	a5,2000
  96:	02f51463          	bne	a0,a5,be <main+0xbe>
      for(i = 0; i < N; i++){
  9a:	34fd                	addiw	s1,s1,-1
  9c:	f4ed                	bnez	s1,86 <main+0x86>
      exit(0);
  9e:	4501                	li	a0,0
  a0:	2f8000ef          	jal	398 <exit>
        printf("%s: create %s failed\n", argv[0], argv[i]);
  a4:	000a3603          	ld	a2,0(s4)
  a8:	0009b583          	ld	a1,0(s3)
  ac:	00001517          	auipc	a0,0x1
  b0:	8ec50513          	addi	a0,a0,-1812 # 998 <malloc+0x118>
  b4:	718000ef          	jal	7cc <printf>
        exit(1);
  b8:	4505                	li	a0,1
  ba:	2de000ef          	jal	398 <exit>
          printf("write failed %d\n", n);
  be:	85aa                	mv	a1,a0
  c0:	00001517          	auipc	a0,0x1
  c4:	8f050513          	addi	a0,a0,-1808 # 9b0 <malloc+0x130>
  c8:	704000ef          	jal	7cc <printf>
          exit(1);
  cc:	4505                	li	a0,1
  ce:	2ca000ef          	jal	398 <exit>
  d2:	893e                	mv	s2,a5
    wait(&xstatus);
  d4:	fcc40513          	addi	a0,s0,-52
  d8:	2c8000ef          	jal	3a0 <wait>
    if(xstatus != 0)
  dc:	fcc42503          	lw	a0,-52(s0)
  e0:	ed09                	bnez	a0,fa <main+0xfa>
  for(int i = 1; i < argc; i++){
  e2:	0019079b          	addiw	a5,s2,1
  e6:	ff2496e3          	bne	s1,s2,d2 <main+0xd2>
      exit(xstatus);
  }
  return 0;
}
  ea:	4501                	li	a0,0
  ec:	70e2                	ld	ra,56(sp)
  ee:	7442                	ld	s0,48(sp)
  f0:	74a2                	ld	s1,40(sp)
  f2:	7902                	ld	s2,32(sp)
  f4:	69e2                	ld	s3,24(sp)
  f6:	6121                	addi	sp,sp,64
  f8:	8082                	ret
  fa:	e852                	sd	s4,16(sp)
      exit(xstatus);
  fc:	29c000ef          	jal	398 <exit>
}
 100:	4501                	li	a0,0
 102:	8082                	ret

0000000000000104 <start>:
//
// wrapper so that it's OK if main() does not call exit().
//
void
start()
{
 104:	1141                	addi	sp,sp,-16
 106:	e406                	sd	ra,8(sp)
 108:	e022                	sd	s0,0(sp)
 10a:	0800                	addi	s0,sp,16
  int r;
  extern int main();
  r = main();
 10c:	ef5ff0ef          	jal	0 <main>
  exit(r);
 110:	288000ef          	jal	398 <exit>

0000000000000114 <strcpy>:
}

char*
strcpy(char *s, const char *t)
{
 114:	1141                	addi	sp,sp,-16
 116:	e422                	sd	s0,8(sp)
 118:	0800                	addi	s0,sp,16
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
 11a:	87aa                	mv	a5,a0
 11c:	0585                	addi	a1,a1,1
 11e:	0785                	addi	a5,a5,1
 120:	fff5c703          	lbu	a4,-1(a1)
 124:	fee78fa3          	sb	a4,-1(a5)
 128:	fb75                	bnez	a4,11c <strcpy+0x8>
    ;
  return os;
}
 12a:	6422                	ld	s0,8(sp)
 12c:	0141                	addi	sp,sp,16
 12e:	8082                	ret

0000000000000130 <strcmp>:

int
strcmp(const char *p, const char *q)
{
 130:	1141                	addi	sp,sp,-16
 132:	e422                	sd	s0,8(sp)
 134:	0800                	addi	s0,sp,16
  while(*p && *p == *q)
 136:	00054783          	lbu	a5,0(a0)
 13a:	cb91                	beqz	a5,14e <strcmp+0x1e>
 13c:	0005c703          	lbu	a4,0(a1)
 140:	00f71763          	bne	a4,a5,14e <strcmp+0x1e>
    p++, q++;
 144:	0505                	addi	a0,a0,1
 146:	0585                	addi	a1,a1,1
  while(*p && *p == *q)
 148:	00054783          	lbu	a5,0(a0)
 14c:	fbe5                	bnez	a5,13c <strcmp+0xc>
  return (uchar)*p - (uchar)*q;
 14e:	0005c503          	lbu	a0,0(a1)
}
 152:	40a7853b          	subw	a0,a5,a0
 156:	6422                	ld	s0,8(sp)
 158:	0141                	addi	sp,sp,16
 15a:	8082                	ret

000000000000015c <strlen>:

uint
strlen(const char *s)
{
 15c:	1141                	addi	sp,sp,-16
 15e:	e422                	sd	s0,8(sp)
 160:	0800                	addi	s0,sp,16
  int n;

  for(n = 0; s[n]; n++)
 162:	00054783          	lbu	a5,0(a0)
 166:	cf91                	beqz	a5,182 <strlen+0x26>
 168:	0505                	addi	a0,a0,1
 16a:	87aa                	mv	a5,a0
 16c:	86be                	mv	a3,a5
 16e:	0785                	addi	a5,a5,1
 170:	fff7c703          	lbu	a4,-1(a5)
 174:	ff65                	bnez	a4,16c <strlen+0x10>
 176:	40a6853b          	subw	a0,a3,a0
 17a:	2505                	addiw	a0,a0,1
    ;
  return n;
}
 17c:	6422                	ld	s0,8(sp)
 17e:	0141                	addi	sp,sp,16
 180:	8082                	ret
  for(n = 0; s[n]; n++)
 182:	4501                	li	a0,0
 184:	bfe5                	j	17c <strlen+0x20>

0000000000000186 <memset>:

void*
memset(void *dst, int c, uint n)
{
 186:	1141                	addi	sp,sp,-16
 188:	e422                	sd	s0,8(sp)
 18a:	0800                	addi	s0,sp,16
  char *cdst = (char *) dst;
  int i;
  for(i = 0; i < n; i++){
 18c:	ca19                	beqz	a2,1a2 <memset+0x1c>
 18e:	87aa                	mv	a5,a0
 190:	1602                	slli	a2,a2,0x20
 192:	9201                	srli	a2,a2,0x20
 194:	00a60733          	add	a4,a2,a0
    cdst[i] = c;
 198:	00b78023          	sb	a1,0(a5)
  for(i = 0; i < n; i++){
 19c:	0785                	addi	a5,a5,1
 19e:	fee79de3          	bne	a5,a4,198 <memset+0x12>
  }
  return dst;
}
 1a2:	6422                	ld	s0,8(sp)
 1a4:	0141                	addi	sp,sp,16
 1a6:	8082                	ret

00000000000001a8 <strchr>:

char*
strchr(const char *s, char c)
{
 1a8:	1141                	addi	sp,sp,-16
 1aa:	e422                	sd	s0,8(sp)
 1ac:	0800                	addi	s0,sp,16
  for(; *s; s++)
 1ae:	00054783          	lbu	a5,0(a0)
 1b2:	cb99                	beqz	a5,1c8 <strchr+0x20>
    if(*s == c)
 1b4:	00f58763          	beq	a1,a5,1c2 <strchr+0x1a>
  for(; *s; s++)
 1b8:	0505                	addi	a0,a0,1
 1ba:	00054783          	lbu	a5,0(a0)
 1be:	fbfd                	bnez	a5,1b4 <strchr+0xc>
      return (char*)s;
  return 0;
 1c0:	4501                	li	a0,0
}
 1c2:	6422                	ld	s0,8(sp)
 1c4:	0141                	addi	sp,sp,16
 1c6:	8082                	ret
  return 0;
 1c8:	4501                	li	a0,0
 1ca:	bfe5                	j	1c2 <strchr+0x1a>

00000000000001cc <gets>:

char*
gets(char *buf, int max)
{
 1cc:	711d                	addi	sp,sp,-96
 1ce:	ec86                	sd	ra,88(sp)
 1d0:	e8a2                	sd	s0,80(sp)
 1d2:	e4a6                	sd	s1,72(sp)
 1d4:	e0ca                	sd	s2,64(sp)
 1d6:	fc4e                	sd	s3,56(sp)
 1d8:	f852                	sd	s4,48(sp)
 1da:	f456                	sd	s5,40(sp)
 1dc:	f05a                	sd	s6,32(sp)
 1de:	ec5e                	sd	s7,24(sp)
 1e0:	1080                	addi	s0,sp,96
 1e2:	8baa                	mv	s7,a0
 1e4:	8a2e                	mv	s4,a1
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
 1e6:	892a                	mv	s2,a0
 1e8:	4481                	li	s1,0
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
 1ea:	4aa9                	li	s5,10
 1ec:	4b35                	li	s6,13
  for(i=0; i+1 < max; ){
 1ee:	89a6                	mv	s3,s1
 1f0:	2485                	addiw	s1,s1,1
 1f2:	0344d663          	bge	s1,s4,21e <gets+0x52>
    cc = read(0, &c, 1);
 1f6:	4605                	li	a2,1
 1f8:	faf40593          	addi	a1,s0,-81
 1fc:	4501                	li	a0,0
 1fe:	1b2000ef          	jal	3b0 <read>
    if(cc < 1)
 202:	00a05e63          	blez	a0,21e <gets+0x52>
    buf[i++] = c;
 206:	faf44783          	lbu	a5,-81(s0)
 20a:	00f90023          	sb	a5,0(s2)
    if(c == '\n' || c == '\r')
 20e:	01578763          	beq	a5,s5,21c <gets+0x50>
 212:	0905                	addi	s2,s2,1
 214:	fd679de3          	bne	a5,s6,1ee <gets+0x22>
    buf[i++] = c;
 218:	89a6                	mv	s3,s1
 21a:	a011                	j	21e <gets+0x52>
 21c:	89a6                	mv	s3,s1
      break;
  }
  buf[i] = '\0';
 21e:	99de                	add	s3,s3,s7
 220:	00098023          	sb	zero,0(s3)
  return buf;
}
 224:	855e                	mv	a0,s7
 226:	60e6                	ld	ra,88(sp)
 228:	6446                	ld	s0,80(sp)
 22a:	64a6                	ld	s1,72(sp)
 22c:	6906                	ld	s2,64(sp)
 22e:	79e2                	ld	s3,56(sp)
 230:	7a42                	ld	s4,48(sp)
 232:	7aa2                	ld	s5,40(sp)
 234:	7b02                	ld	s6,32(sp)
 236:	6be2                	ld	s7,24(sp)
 238:	6125                	addi	sp,sp,96
 23a:	8082                	ret

000000000000023c <stat>:

int
stat(const char *n, struct stat *st)
{
 23c:	1101                	addi	sp,sp,-32
 23e:	ec06                	sd	ra,24(sp)
 240:	e822                	sd	s0,16(sp)
 242:	e04a                	sd	s2,0(sp)
 244:	1000                	addi	s0,sp,32
 246:	892e                	mv	s2,a1
  int fd;
  int r;

  fd = open(n, O_RDONLY);
 248:	4581                	li	a1,0
 24a:	18e000ef          	jal	3d8 <open>
  if(fd < 0)
 24e:	02054263          	bltz	a0,272 <stat+0x36>
 252:	e426                	sd	s1,8(sp)
 254:	84aa                	mv	s1,a0
    return -1;
  r = fstat(fd, st);
 256:	85ca                	mv	a1,s2
 258:	198000ef          	jal	3f0 <fstat>
 25c:	892a                	mv	s2,a0
  close(fd);
 25e:	8526                	mv	a0,s1
 260:	160000ef          	jal	3c0 <close>
  return r;
 264:	64a2                	ld	s1,8(sp)
}
 266:	854a                	mv	a0,s2
 268:	60e2                	ld	ra,24(sp)
 26a:	6442                	ld	s0,16(sp)
 26c:	6902                	ld	s2,0(sp)
 26e:	6105                	addi	sp,sp,32
 270:	8082                	ret
    return -1;
 272:	597d                	li	s2,-1
 274:	bfcd                	j	266 <stat+0x2a>

0000000000000276 <atoi>:

int
atoi(const char *s)
{
 276:	1141                	addi	sp,sp,-16
 278:	e422                	sd	s0,8(sp)
 27a:	0800                	addi	s0,sp,16
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
 27c:	00054683          	lbu	a3,0(a0)
 280:	fd06879b          	addiw	a5,a3,-48
 284:	0ff7f793          	zext.b	a5,a5
 288:	4625                	li	a2,9
 28a:	02f66863          	bltu	a2,a5,2ba <atoi+0x44>
 28e:	872a                	mv	a4,a0
  n = 0;
 290:	4501                	li	a0,0
    n = n*10 + *s++ - '0';
 292:	0705                	addi	a4,a4,1
 294:	0025179b          	slliw	a5,a0,0x2
 298:	9fa9                	addw	a5,a5,a0
 29a:	0017979b          	slliw	a5,a5,0x1
 29e:	9fb5                	addw	a5,a5,a3
 2a0:	fd07851b          	addiw	a0,a5,-48
  while('0' <= *s && *s <= '9')
 2a4:	00074683          	lbu	a3,0(a4)
 2a8:	fd06879b          	addiw	a5,a3,-48
 2ac:	0ff7f793          	zext.b	a5,a5
 2b0:	fef671e3          	bgeu	a2,a5,292 <atoi+0x1c>
  return n;
}
 2b4:	6422                	ld	s0,8(sp)
 2b6:	0141                	addi	sp,sp,16
 2b8:	8082                	ret
  n = 0;
 2ba:	4501                	li	a0,0
 2bc:	bfe5                	j	2b4 <atoi+0x3e>

00000000000002be <memmove>:

void*
memmove(void *vdst, const void *vsrc, int n)
{
 2be:	1141                	addi	sp,sp,-16
 2c0:	e422                	sd	s0,8(sp)
 2c2:	0800                	addi	s0,sp,16
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  if (src > dst) {
 2c4:	02b57463          	bgeu	a0,a1,2ec <memmove+0x2e>
    while(n-- > 0)
 2c8:	00c05f63          	blez	a2,2e6 <memmove+0x28>
 2cc:	1602                	slli	a2,a2,0x20
 2ce:	9201                	srli	a2,a2,0x20
 2d0:	00c507b3          	add	a5,a0,a2
  dst = vdst;
 2d4:	872a                	mv	a4,a0
      *dst++ = *src++;
 2d6:	0585                	addi	a1,a1,1
 2d8:	0705                	addi	a4,a4,1
 2da:	fff5c683          	lbu	a3,-1(a1)
 2de:	fed70fa3          	sb	a3,-1(a4)
    while(n-- > 0)
 2e2:	fef71ae3          	bne	a4,a5,2d6 <memmove+0x18>
    src += n;
    while(n-- > 0)
      *--dst = *--src;
  }
  return vdst;
}
 2e6:	6422                	ld	s0,8(sp)
 2e8:	0141                	addi	sp,sp,16
 2ea:	8082                	ret
    dst += n;
 2ec:	00c50733          	add	a4,a0,a2
    src += n;
 2f0:	95b2                	add	a1,a1,a2
    while(n-- > 0)
 2f2:	fec05ae3          	blez	a2,2e6 <memmove+0x28>
 2f6:	fff6079b          	addiw	a5,a2,-1
 2fa:	1782                	slli	a5,a5,0x20
 2fc:	9381                	srli	a5,a5,0x20
 2fe:	fff7c793          	not	a5,a5
 302:	97ba                	add	a5,a5,a4
      *--dst = *--src;
 304:	15fd                	addi	a1,a1,-1
 306:	177d                	addi	a4,a4,-1
 308:	0005c683          	lbu	a3,0(a1)
 30c:	00d70023          	sb	a3,0(a4)
    while(n-- > 0)
 310:	fee79ae3          	bne	a5,a4,304 <memmove+0x46>
 314:	bfc9                	j	2e6 <memmove+0x28>

0000000000000316 <memcmp>:

int
memcmp(const void *s1, const void *s2, uint n)
{
 316:	1141                	addi	sp,sp,-16
 318:	e422                	sd	s0,8(sp)
 31a:	0800                	addi	s0,sp,16
  const char *p1 = s1, *p2 = s2;
  while (n-- > 0) {
 31c:	ca05                	beqz	a2,34c <memcmp+0x36>
 31e:	fff6069b          	addiw	a3,a2,-1
 322:	1682                	slli	a3,a3,0x20
 324:	9281                	srli	a3,a3,0x20
 326:	0685                	addi	a3,a3,1
 328:	96aa                	add	a3,a3,a0
    if (*p1 != *p2) {
 32a:	00054783          	lbu	a5,0(a0)
 32e:	0005c703          	lbu	a4,0(a1)
 332:	00e79863          	bne	a5,a4,342 <memcmp+0x2c>
      return *p1 - *p2;
    }
    p1++;
 336:	0505                	addi	a0,a0,1
    p2++;
 338:	0585                	addi	a1,a1,1
  while (n-- > 0) {
 33a:	fed518e3          	bne	a0,a3,32a <memcmp+0x14>
  }
  return 0;
 33e:	4501                	li	a0,0
 340:	a019                	j	346 <memcmp+0x30>
      return *p1 - *p2;
 342:	40e7853b          	subw	a0,a5,a4
}
 346:	6422                	ld	s0,8(sp)
 348:	0141                	addi	sp,sp,16
 34a:	8082                	ret
  return 0;
 34c:	4501                	li	a0,0
 34e:	bfe5                	j	346 <memcmp+0x30>

0000000000000350 <memcpy>:

void *
memcpy(void *dst, const void *src, uint n)
{
 350:	1141                	addi	sp,sp,-16
 352:	e406                	sd	ra,8(sp)
 354:	e022                	sd	s0,0(sp)
 356:	0800                	addi	s0,sp,16
  return memmove(dst, src, n);
 358:	f67ff0ef          	jal	2be <memmove>
}
 35c:	60a2                	ld	ra,8(sp)
 35e:	6402                	ld	s0,0(sp)
 360:	0141                	addi	sp,sp,16
 362:	8082                	ret

0000000000000364 <sbrk>:

char *
sbrk(int n) {
 364:	1141                	addi	sp,sp,-16
 366:	e406                	sd	ra,8(sp)
 368:	e022                	sd	s0,0(sp)
 36a:	0800                	addi	s0,sp,16
  return sys_sbrk(n, SBRK_EAGER);
 36c:	4585                	li	a1,1
 36e:	0b2000ef          	jal	420 <sys_sbrk>
}
 372:	60a2                	ld	ra,8(sp)
 374:	6402                	ld	s0,0(sp)
 376:	0141                	addi	sp,sp,16
 378:	8082                	ret

000000000000037a <sbrklazy>:

char *
sbrklazy(int n) {
 37a:	1141                	addi	sp,sp,-16
 37c:	e406                	sd	ra,8(sp)
 37e:	e022                	sd	s0,0(sp)
 380:	0800                	addi	s0,sp,16
  return sys_sbrk(n, SBRK_LAZY);
 382:	4589                	li	a1,2
 384:	09c000ef          	jal	420 <sys_sbrk>
}
 388:	60a2                	ld	ra,8(sp)
 38a:	6402                	ld	s0,0(sp)
 38c:	0141                	addi	sp,sp,16
 38e:	8082                	ret

0000000000000390 <fork>:
# generated by usys.pl - do not edit
#include "kernel/syscall.h"
.global fork
fork:
 li a7, SYS_fork
 390:	4885                	li	a7,1
 ecall
 392:	00000073          	ecall
 ret
 396:	8082                	ret

0000000000000398 <exit>:
.global exit
exit:
 li a7, SYS_exit
 398:	4889                	li	a7,2
 ecall
 39a:	00000073          	ecall
 ret
 39e:	8082                	ret

00000000000003a0 <wait>:
.global wait
wait:
 li a7, SYS_wait
 3a0:	488d                	li	a7,3
 ecall
 3a2:	00000073          	ecall
 ret
 3a6:	8082                	ret

00000000000003a8 <pipe>:
.global pipe
pipe:
 li a7, SYS_pipe
 3a8:	4891                	li	a7,4
 ecall
 3aa:	00000073          	ecall
 ret
 3ae:	8082                	ret

00000000000003b0 <read>:
.global read
read:
 li a7, SYS_read
 3b0:	4895                	li	a7,5
 ecall
 3b2:	00000073          	ecall
 ret
 3b6:	8082                	ret

00000000000003b8 <write>:
.global write
write:
 li a7, SYS_write
 3b8:	48c1                	li	a7,16
 ecall
 3ba:	00000073          	ecall
 ret
 3be:	8082                	ret

00000000000003c0 <close>:
.global close
close:
 li a7, SYS_close
 3c0:	48d5                	li	a7,21
 ecall
 3c2:	00000073          	ecall
 ret
 3c6:	8082                	ret

00000000000003c8 <kill>:
.global kill
kill:
 li a7, SYS_kill
 3c8:	4899                	li	a7,6
 ecall
 3ca:	00000073          	ecall
 ret
 3ce:	8082                	ret

00000000000003d0 <exec>:
.global exec
exec:
 li a7, SYS_exec
 3d0:	489d                	li	a7,7
 ecall
 3d2:	00000073          	ecall
 ret
 3d6:	8082                	ret

00000000000003d8 <open>:
.global open
open:
 li a7, SYS_open
 3d8:	48bd                	li	a7,15
 ecall
 3da:	00000073          	ecall
 ret
 3de:	8082                	ret

00000000000003e0 <mknod>:
.global mknod
mknod:
 li a7, SYS_mknod
 3e0:	48c5                	li	a7,17
 ecall
 3e2:	00000073          	ecall
 ret
 3e6:	8082                	ret

00000000000003e8 <unlink>:
.global unlink
unlink:
 li a7, SYS_unlink
 3e8:	48c9                	li	a7,18
 ecall
 3ea:	00000073          	ecall
 ret
 3ee:	8082                	ret

00000000000003f0 <fstat>:
.global fstat
fstat:
 li a7, SYS_fstat
 3f0:	48a1                	li	a7,8
 ecall
 3f2:	00000073          	ecall
 ret
 3f6:	8082                	ret

00000000000003f8 <link>:
.global link
link:
 li a7, SYS_link
 3f8:	48cd                	li	a7,19
 ecall
 3fa:	00000073          	ecall
 ret
 3fe:	8082                	ret

0000000000000400 <mkdir>:
.global mkdir
mkdir:
 li a7, SYS_mkdir
 400:	48d1                	li	a7,20
 ecall
 402:	00000073          	ecall
 ret
 406:	8082                	ret

0000000000000408 <chdir>:
.global chdir
chdir:
 li a7, SYS_chdir
 408:	48a5                	li	a7,9
 ecall
 40a:	00000073          	ecall
 ret
 40e:	8082                	ret

0000000000000410 <dup>:
.global dup
dup:
 li a7, SYS_dup
 410:	48a9                	li	a7,10
 ecall
 412:	00000073          	ecall
 ret
 416:	8082                	ret

0000000000000418 <getpid>:
.global getpid
getpid:
 li a7, SYS_getpid
 418:	48ad                	li	a7,11
 ecall
 41a:	00000073          	ecall
 ret
 41e:	8082                	ret

0000000000000420 <sys_sbrk>:
.global sys_sbrk
sys_sbrk:
 li a7, SYS_sbrk
 420:	48b1                	li	a7,12
 ecall
 422:	00000073          	ecall
 ret
 426:	8082                	ret

0000000000000428 <pause>:
.global pause
pause:
 li a7, SYS_pause
 428:	48b5                	li	a7,13
 ecall
 42a:	00000073          	ecall
 ret
 42e:	8082                	ret

0000000000000430 <uptime>:
.global uptime
uptime:
 li a7, SYS_uptime
 430:	48b9                	li	a7,14
 ecall
 432:	00000073          	ecall
 ret
 436:	8082                	ret

0000000000000438 <putc>:

static char digits[] = "0123456789ABCDEF";

static void
putc(int fd, char c)
{
 438:	1101                	addi	sp,sp,-32
 43a:	ec06                	sd	ra,24(sp)
 43c:	e822                	sd	s0,16(sp)
 43e:	1000                	addi	s0,sp,32
 440:	feb407a3          	sb	a1,-17(s0)
  write(fd, &c, 1);
 444:	4605                	li	a2,1
 446:	fef40593          	addi	a1,s0,-17
 44a:	f6fff0ef          	jal	3b8 <write>
}
 44e:	60e2                	ld	ra,24(sp)
 450:	6442                	ld	s0,16(sp)
 452:	6105                	addi	sp,sp,32
 454:	8082                	ret

0000000000000456 <printint>:

static void
printint(int fd, long long xx, int base, int sgn)
{
 456:	715d                	addi	sp,sp,-80
 458:	e486                	sd	ra,72(sp)
 45a:	e0a2                	sd	s0,64(sp)
 45c:	fc26                	sd	s1,56(sp)
 45e:	0880                	addi	s0,sp,80
 460:	84aa                	mv	s1,a0
  char buf[20];
  int i, neg;
  uint x;

  neg = 0;
  if(sgn && xx < 0){
 462:	c299                	beqz	a3,468 <printint+0x12>
 464:	0805c963          	bltz	a1,4f6 <printint+0xa0>
    neg = 1;
    x = -xx;
  } else {
    x = xx;
 468:	2581                	sext.w	a1,a1
  neg = 0;
 46a:	4881                	li	a7,0
 46c:	fb840693          	addi	a3,s0,-72
  }

  i = 0;
 470:	4701                	li	a4,0
  do{
    buf[i++] = digits[x % base];
 472:	2601                	sext.w	a2,a2
 474:	00000517          	auipc	a0,0x0
 478:	55c50513          	addi	a0,a0,1372 # 9d0 <digits>
 47c:	883a                	mv	a6,a4
 47e:	2705                	addiw	a4,a4,1
 480:	02c5f7bb          	remuw	a5,a1,a2
 484:	1782                	slli	a5,a5,0x20
 486:	9381                	srli	a5,a5,0x20
 488:	97aa                	add	a5,a5,a0
 48a:	0007c783          	lbu	a5,0(a5)
 48e:	00f68023          	sb	a5,0(a3)
  }while((x /= base) != 0);
 492:	0005879b          	sext.w	a5,a1
 496:	02c5d5bb          	divuw	a1,a1,a2
 49a:	0685                	addi	a3,a3,1
 49c:	fec7f0e3          	bgeu	a5,a2,47c <printint+0x26>
  if(neg)
 4a0:	00088c63          	beqz	a7,4b8 <printint+0x62>
    buf[i++] = '-';
 4a4:	fd070793          	addi	a5,a4,-48
 4a8:	00878733          	add	a4,a5,s0
 4ac:	02d00793          	li	a5,45
 4b0:	fef70423          	sb	a5,-24(a4)
 4b4:	0028071b          	addiw	a4,a6,2

  while(--i >= 0)
 4b8:	02e05a63          	blez	a4,4ec <printint+0x96>
 4bc:	f84a                	sd	s2,48(sp)
 4be:	f44e                	sd	s3,40(sp)
 4c0:	fb840793          	addi	a5,s0,-72
 4c4:	00e78933          	add	s2,a5,a4
 4c8:	fff78993          	addi	s3,a5,-1
 4cc:	99ba                	add	s3,s3,a4
 4ce:	377d                	addiw	a4,a4,-1
 4d0:	1702                	slli	a4,a4,0x20
 4d2:	9301                	srli	a4,a4,0x20
 4d4:	40e989b3          	sub	s3,s3,a4
    putc(fd, buf[i]);
 4d8:	fff94583          	lbu	a1,-1(s2)
 4dc:	8526                	mv	a0,s1
 4de:	f5bff0ef          	jal	438 <putc>
  while(--i >= 0)
 4e2:	197d                	addi	s2,s2,-1
 4e4:	ff391ae3          	bne	s2,s3,4d8 <printint+0x82>
 4e8:	7942                	ld	s2,48(sp)
 4ea:	79a2                	ld	s3,40(sp)
}
 4ec:	60a6                	ld	ra,72(sp)
 4ee:	6406                	ld	s0,64(sp)
 4f0:	74e2                	ld	s1,56(sp)
 4f2:	6161                	addi	sp,sp,80
 4f4:	8082                	ret
    x = -xx;
 4f6:	40b005bb          	negw	a1,a1
    neg = 1;
 4fa:	4885                	li	a7,1
    x = -xx;
 4fc:	bf85                	j	46c <printint+0x16>

00000000000004fe <vprintf>:
}

// Print to the given fd. Only understands %d, %x, %p, %c, %s.
void
vprintf(int fd, const char *fmt, va_list ap)
{
 4fe:	711d                	addi	sp,sp,-96
 500:	ec86                	sd	ra,88(sp)
 502:	e8a2                	sd	s0,80(sp)
 504:	e0ca                	sd	s2,64(sp)
 506:	1080                	addi	s0,sp,96
  char *s;
  int c0, c1, c2, i, state;

  state = 0;
  for(i = 0; fmt[i]; i++){
 508:	0005c903          	lbu	s2,0(a1)
 50c:	28090663          	beqz	s2,798 <vprintf+0x29a>
 510:	e4a6                	sd	s1,72(sp)
 512:	fc4e                	sd	s3,56(sp)
 514:	f852                	sd	s4,48(sp)
 516:	f456                	sd	s5,40(sp)
 518:	f05a                	sd	s6,32(sp)
 51a:	ec5e                	sd	s7,24(sp)
 51c:	e862                	sd	s8,16(sp)
 51e:	e466                	sd	s9,8(sp)
 520:	8b2a                	mv	s6,a0
 522:	8a2e                	mv	s4,a1
 524:	8bb2                	mv	s7,a2
  state = 0;
 526:	4981                	li	s3,0
  for(i = 0; fmt[i]; i++){
 528:	4481                	li	s1,0
 52a:	4701                	li	a4,0
      if(c0 == '%'){
        state = '%';
      } else {
        putc(fd, c0);
      }
    } else if(state == '%'){
 52c:	02500a93          	li	s5,37
      c1 = c2 = 0;
      if(c0) c1 = fmt[i+1] & 0xff;
      if(c1) c2 = fmt[i+2] & 0xff;
      if(c0 == 'd'){
 530:	06400c13          	li	s8,100
        printint(fd, va_arg(ap, int), 10, 1);
      } else if(c0 == 'l' && c1 == 'd'){
 534:	06c00c93          	li	s9,108
 538:	a005                	j	558 <vprintf+0x5a>
        putc(fd, c0);
 53a:	85ca                	mv	a1,s2
 53c:	855a                	mv	a0,s6
 53e:	efbff0ef          	jal	438 <putc>
 542:	a019                	j	548 <vprintf+0x4a>
    } else if(state == '%'){
 544:	03598263          	beq	s3,s5,568 <vprintf+0x6a>
  for(i = 0; fmt[i]; i++){
 548:	2485                	addiw	s1,s1,1
 54a:	8726                	mv	a4,s1
 54c:	009a07b3          	add	a5,s4,s1
 550:	0007c903          	lbu	s2,0(a5)
 554:	22090a63          	beqz	s2,788 <vprintf+0x28a>
    c0 = fmt[i] & 0xff;
 558:	0009079b          	sext.w	a5,s2
    if(state == 0){
 55c:	fe0994e3          	bnez	s3,544 <vprintf+0x46>
      if(c0 == '%'){
 560:	fd579de3          	bne	a5,s5,53a <vprintf+0x3c>
        state = '%';
 564:	89be                	mv	s3,a5
 566:	b7cd                	j	548 <vprintf+0x4a>
      if(c0) c1 = fmt[i+1] & 0xff;
 568:	00ea06b3          	add	a3,s4,a4
 56c:	0016c683          	lbu	a3,1(a3)
      c1 = c2 = 0;
 570:	8636                	mv	a2,a3
      if(c1) c2 = fmt[i+2] & 0xff;
 572:	c681                	beqz	a3,57a <vprintf+0x7c>
 574:	9752                	add	a4,a4,s4
 576:	00274603          	lbu	a2,2(a4)
      if(c0 == 'd'){
 57a:	05878363          	beq	a5,s8,5c0 <vprintf+0xc2>
      } else if(c0 == 'l' && c1 == 'd'){
 57e:	05978d63          	beq	a5,s9,5d8 <vprintf+0xda>
        printint(fd, va_arg(ap, uint64), 10, 1);
        i += 1;
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'd'){
        printint(fd, va_arg(ap, uint64), 10, 1);
        i += 2;
      } else if(c0 == 'u'){
 582:	07500713          	li	a4,117
 586:	0ee78763          	beq	a5,a4,674 <vprintf+0x176>
        printint(fd, va_arg(ap, uint64), 10, 0);
        i += 1;
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'u'){
        printint(fd, va_arg(ap, uint64), 10, 0);
        i += 2;
      } else if(c0 == 'x'){
 58a:	07800713          	li	a4,120
 58e:	12e78963          	beq	a5,a4,6c0 <vprintf+0x1c2>
        printint(fd, va_arg(ap, uint64), 16, 0);
        i += 1;
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'x'){
        printint(fd, va_arg(ap, uint64), 16, 0);
        i += 2;
      } else if(c0 == 'p'){
 592:	07000713          	li	a4,112
 596:	14e78e63          	beq	a5,a4,6f2 <vprintf+0x1f4>
        printptr(fd, va_arg(ap, uint64));
      } else if(c0 == 'c'){
 59a:	06300713          	li	a4,99
 59e:	18e78e63          	beq	a5,a4,73a <vprintf+0x23c>
        putc(fd, va_arg(ap, uint32));
      } else if(c0 == 's'){
 5a2:	07300713          	li	a4,115
 5a6:	1ae78463          	beq	a5,a4,74e <vprintf+0x250>
        if((s = va_arg(ap, char*)) == 0)
          s = "(null)";
        for(; *s; s++)
          putc(fd, *s);
      } else if(c0 == '%'){
 5aa:	02500713          	li	a4,37
 5ae:	04e79563          	bne	a5,a4,5f8 <vprintf+0xfa>
        putc(fd, '%');
 5b2:	02500593          	li	a1,37
 5b6:	855a                	mv	a0,s6
 5b8:	e81ff0ef          	jal	438 <putc>
        // Unknown % sequence.  Print it to draw attention.
        putc(fd, '%');
        putc(fd, c0);
      }

      state = 0;
 5bc:	4981                	li	s3,0
 5be:	b769                	j	548 <vprintf+0x4a>
        printint(fd, va_arg(ap, int), 10, 1);
 5c0:	008b8913          	addi	s2,s7,8
 5c4:	4685                	li	a3,1
 5c6:	4629                	li	a2,10
 5c8:	000ba583          	lw	a1,0(s7)
 5cc:	855a                	mv	a0,s6
 5ce:	e89ff0ef          	jal	456 <printint>
 5d2:	8bca                	mv	s7,s2
      state = 0;
 5d4:	4981                	li	s3,0
 5d6:	bf8d                	j	548 <vprintf+0x4a>
      } else if(c0 == 'l' && c1 == 'd'){
 5d8:	06400793          	li	a5,100
 5dc:	02f68963          	beq	a3,a5,60e <vprintf+0x110>
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'd'){
 5e0:	06c00793          	li	a5,108
 5e4:	04f68263          	beq	a3,a5,628 <vprintf+0x12a>
      } else if(c0 == 'l' && c1 == 'u'){
 5e8:	07500793          	li	a5,117
 5ec:	0af68063          	beq	a3,a5,68c <vprintf+0x18e>
      } else if(c0 == 'l' && c1 == 'x'){
 5f0:	07800793          	li	a5,120
 5f4:	0ef68263          	beq	a3,a5,6d8 <vprintf+0x1da>
        putc(fd, '%');
 5f8:	02500593          	li	a1,37
 5fc:	855a                	mv	a0,s6
 5fe:	e3bff0ef          	jal	438 <putc>
        putc(fd, c0);
 602:	85ca                	mv	a1,s2
 604:	855a                	mv	a0,s6
 606:	e33ff0ef          	jal	438 <putc>
      state = 0;
 60a:	4981                	li	s3,0
 60c:	bf35                	j	548 <vprintf+0x4a>
        printint(fd, va_arg(ap, uint64), 10, 1);
 60e:	008b8913          	addi	s2,s7,8
 612:	4685                	li	a3,1
 614:	4629                	li	a2,10
 616:	000bb583          	ld	a1,0(s7)
 61a:	855a                	mv	a0,s6
 61c:	e3bff0ef          	jal	456 <printint>
        i += 1;
 620:	2485                	addiw	s1,s1,1
        printint(fd, va_arg(ap, uint64), 10, 1);
 622:	8bca                	mv	s7,s2
      state = 0;
 624:	4981                	li	s3,0
        i += 1;
 626:	b70d                	j	548 <vprintf+0x4a>
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'd'){
 628:	06400793          	li	a5,100
 62c:	02f60763          	beq	a2,a5,65a <vprintf+0x15c>
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'u'){
 630:	07500793          	li	a5,117
 634:	06f60963          	beq	a2,a5,6a6 <vprintf+0x1a8>
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'x'){
 638:	07800793          	li	a5,120
 63c:	faf61ee3          	bne	a2,a5,5f8 <vprintf+0xfa>
        printint(fd, va_arg(ap, uint64), 16, 0);
 640:	008b8913          	addi	s2,s7,8
 644:	4681                	li	a3,0
 646:	4641                	li	a2,16
 648:	000bb583          	ld	a1,0(s7)
 64c:	855a                	mv	a0,s6
 64e:	e09ff0ef          	jal	456 <printint>
        i += 2;
 652:	2489                	addiw	s1,s1,2
        printint(fd, va_arg(ap, uint64), 16, 0);
 654:	8bca                	mv	s7,s2
      state = 0;
 656:	4981                	li	s3,0
        i += 2;
 658:	bdc5                	j	548 <vprintf+0x4a>
        printint(fd, va_arg(ap, uint64), 10, 1);
 65a:	008b8913          	addi	s2,s7,8
 65e:	4685                	li	a3,1
 660:	4629                	li	a2,10
 662:	000bb583          	ld	a1,0(s7)
 666:	855a                	mv	a0,s6
 668:	defff0ef          	jal	456 <printint>
        i += 2;
 66c:	2489                	addiw	s1,s1,2
        printint(fd, va_arg(ap, uint64), 10, 1);
 66e:	8bca                	mv	s7,s2
      state = 0;
 670:	4981                	li	s3,0
        i += 2;
 672:	bdd9                	j	548 <vprintf+0x4a>
        printint(fd, va_arg(ap, uint32), 10, 0);
 674:	008b8913          	addi	s2,s7,8
 678:	4681                	li	a3,0
 67a:	4629                	li	a2,10
 67c:	000be583          	lwu	a1,0(s7)
 680:	855a                	mv	a0,s6
 682:	dd5ff0ef          	jal	456 <printint>
 686:	8bca                	mv	s7,s2
      state = 0;
 688:	4981                	li	s3,0
 68a:	bd7d                	j	548 <vprintf+0x4a>
        printint(fd, va_arg(ap, uint64), 10, 0);
 68c:	008b8913          	addi	s2,s7,8
 690:	4681                	li	a3,0
 692:	4629                	li	a2,10
 694:	000bb583          	ld	a1,0(s7)
 698:	855a                	mv	a0,s6
 69a:	dbdff0ef          	jal	456 <printint>
        i += 1;
 69e:	2485                	addiw	s1,s1,1
        printint(fd, va_arg(ap, uint64), 10, 0);
 6a0:	8bca                	mv	s7,s2
      state = 0;
 6a2:	4981                	li	s3,0
        i += 1;
 6a4:	b555                	j	548 <vprintf+0x4a>
        printint(fd, va_arg(ap, uint64), 10, 0);
 6a6:	008b8913          	addi	s2,s7,8
 6aa:	4681                	li	a3,0
 6ac:	4629                	li	a2,10
 6ae:	000bb583          	ld	a1,0(s7)
 6b2:	855a                	mv	a0,s6
 6b4:	da3ff0ef          	jal	456 <printint>
        i += 2;
 6b8:	2489                	addiw	s1,s1,2
        printint(fd, va_arg(ap, uint64), 10, 0);
 6ba:	8bca                	mv	s7,s2
      state = 0;
 6bc:	4981                	li	s3,0
        i += 2;
 6be:	b569                	j	548 <vprintf+0x4a>
        printint(fd, va_arg(ap, uint32), 16, 0);
 6c0:	008b8913          	addi	s2,s7,8
 6c4:	4681                	li	a3,0
 6c6:	4641                	li	a2,16
 6c8:	000be583          	lwu	a1,0(s7)
 6cc:	855a                	mv	a0,s6
 6ce:	d89ff0ef          	jal	456 <printint>
 6d2:	8bca                	mv	s7,s2
      state = 0;
 6d4:	4981                	li	s3,0
 6d6:	bd8d                	j	548 <vprintf+0x4a>
        printint(fd, va_arg(ap, uint64), 16, 0);
 6d8:	008b8913          	addi	s2,s7,8
 6dc:	4681                	li	a3,0
 6de:	4641                	li	a2,16
 6e0:	000bb583          	ld	a1,0(s7)
 6e4:	855a                	mv	a0,s6
 6e6:	d71ff0ef          	jal	456 <printint>
        i += 1;
 6ea:	2485                	addiw	s1,s1,1
        printint(fd, va_arg(ap, uint64), 16, 0);
 6ec:	8bca                	mv	s7,s2
      state = 0;
 6ee:	4981                	li	s3,0
        i += 1;
 6f0:	bda1                	j	548 <vprintf+0x4a>
 6f2:	e06a                	sd	s10,0(sp)
        printptr(fd, va_arg(ap, uint64));
 6f4:	008b8d13          	addi	s10,s7,8
 6f8:	000bb983          	ld	s3,0(s7)
  putc(fd, '0');
 6fc:	03000593          	li	a1,48
 700:	855a                	mv	a0,s6
 702:	d37ff0ef          	jal	438 <putc>
  putc(fd, 'x');
 706:	07800593          	li	a1,120
 70a:	855a                	mv	a0,s6
 70c:	d2dff0ef          	jal	438 <putc>
 710:	4941                	li	s2,16
    putc(fd, digits[x >> (sizeof(uint64) * 8 - 4)]);
 712:	00000b97          	auipc	s7,0x0
 716:	2beb8b93          	addi	s7,s7,702 # 9d0 <digits>
 71a:	03c9d793          	srli	a5,s3,0x3c
 71e:	97de                	add	a5,a5,s7
 720:	0007c583          	lbu	a1,0(a5)
 724:	855a                	mv	a0,s6
 726:	d13ff0ef          	jal	438 <putc>
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
 72a:	0992                	slli	s3,s3,0x4
 72c:	397d                	addiw	s2,s2,-1
 72e:	fe0916e3          	bnez	s2,71a <vprintf+0x21c>
        printptr(fd, va_arg(ap, uint64));
 732:	8bea                	mv	s7,s10
      state = 0;
 734:	4981                	li	s3,0
 736:	6d02                	ld	s10,0(sp)
 738:	bd01                	j	548 <vprintf+0x4a>
        putc(fd, va_arg(ap, uint32));
 73a:	008b8913          	addi	s2,s7,8
 73e:	000bc583          	lbu	a1,0(s7)
 742:	855a                	mv	a0,s6
 744:	cf5ff0ef          	jal	438 <putc>
 748:	8bca                	mv	s7,s2
      state = 0;
 74a:	4981                	li	s3,0
 74c:	bbf5                	j	548 <vprintf+0x4a>
        if((s = va_arg(ap, char*)) == 0)
 74e:	008b8993          	addi	s3,s7,8
 752:	000bb903          	ld	s2,0(s7)
 756:	00090f63          	beqz	s2,774 <vprintf+0x276>
        for(; *s; s++)
 75a:	00094583          	lbu	a1,0(s2)
 75e:	c195                	beqz	a1,782 <vprintf+0x284>
          putc(fd, *s);
 760:	855a                	mv	a0,s6
 762:	cd7ff0ef          	jal	438 <putc>
        for(; *s; s++)
 766:	0905                	addi	s2,s2,1
 768:	00094583          	lbu	a1,0(s2)
 76c:	f9f5                	bnez	a1,760 <vprintf+0x262>
        if((s = va_arg(ap, char*)) == 0)
 76e:	8bce                	mv	s7,s3
      state = 0;
 770:	4981                	li	s3,0
 772:	bbd9                	j	548 <vprintf+0x4a>
          s = "(null)";
 774:	00000917          	auipc	s2,0x0
 778:	25490913          	addi	s2,s2,596 # 9c8 <malloc+0x148>
        for(; *s; s++)
 77c:	02800593          	li	a1,40
 780:	b7c5                	j	760 <vprintf+0x262>
        if((s = va_arg(ap, char*)) == 0)
 782:	8bce                	mv	s7,s3
      state = 0;
 784:	4981                	li	s3,0
 786:	b3c9                	j	548 <vprintf+0x4a>
 788:	64a6                	ld	s1,72(sp)
 78a:	79e2                	ld	s3,56(sp)
 78c:	7a42                	ld	s4,48(sp)
 78e:	7aa2                	ld	s5,40(sp)
 790:	7b02                	ld	s6,32(sp)
 792:	6be2                	ld	s7,24(sp)
 794:	6c42                	ld	s8,16(sp)
 796:	6ca2                	ld	s9,8(sp)
    }
  }
}
 798:	60e6                	ld	ra,88(sp)
 79a:	6446                	ld	s0,80(sp)
 79c:	6906                	ld	s2,64(sp)
 79e:	6125                	addi	sp,sp,96
 7a0:	8082                	ret

00000000000007a2 <fprintf>:

void
fprintf(int fd, const char *fmt, ...)
{
 7a2:	715d                	addi	sp,sp,-80
 7a4:	ec06                	sd	ra,24(sp)
 7a6:	e822                	sd	s0,16(sp)
 7a8:	1000                	addi	s0,sp,32
 7aa:	e010                	sd	a2,0(s0)
 7ac:	e414                	sd	a3,8(s0)
 7ae:	e818                	sd	a4,16(s0)
 7b0:	ec1c                	sd	a5,24(s0)
 7b2:	03043023          	sd	a6,32(s0)
 7b6:	03143423          	sd	a7,40(s0)
  va_list ap;

  va_start(ap, fmt);
 7ba:	fe843423          	sd	s0,-24(s0)
  vprintf(fd, fmt, ap);
 7be:	8622                	mv	a2,s0
 7c0:	d3fff0ef          	jal	4fe <vprintf>
}
 7c4:	60e2                	ld	ra,24(sp)
 7c6:	6442                	ld	s0,16(sp)
 7c8:	6161                	addi	sp,sp,80
 7ca:	8082                	ret

00000000000007cc <printf>:

void
printf(const char *fmt, ...)
{
 7cc:	711d                	addi	sp,sp,-96
 7ce:	ec06                	sd	ra,24(sp)
 7d0:	e822                	sd	s0,16(sp)
 7d2:	1000                	addi	s0,sp,32
 7d4:	e40c                	sd	a1,8(s0)
 7d6:	e810                	sd	a2,16(s0)
 7d8:	ec14                	sd	a3,24(s0)
 7da:	f018                	sd	a4,32(s0)
 7dc:	f41c                	sd	a5,40(s0)
 7de:	03043823          	sd	a6,48(s0)
 7e2:	03143c23          	sd	a7,56(s0)
  va_list ap;

  va_start(ap, fmt);
 7e6:	00840613          	addi	a2,s0,8
 7ea:	fec43423          	sd	a2,-24(s0)
  vprintf(1, fmt, ap);
 7ee:	85aa                	mv	a1,a0
 7f0:	4505                	li	a0,1
 7f2:	d0dff0ef          	jal	4fe <vprintf>
}
 7f6:	60e2                	ld	ra,24(sp)
 7f8:	6442                	ld	s0,16(sp)
 7fa:	6125                	addi	sp,sp,96
 7fc:	8082                	ret

00000000000007fe <free>:
static Header base;
static Header *freep;

void
free(void *ap)
{
 7fe:	1141                	addi	sp,sp,-16
 800:	e422                	sd	s0,8(sp)
 802:	0800                	addi	s0,sp,16
  Header *bp, *p;

  bp = (Header*)ap - 1;
 804:	ff050693          	addi	a3,a0,-16
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
 808:	00000797          	auipc	a5,0x0
 80c:	7f87b783          	ld	a5,2040(a5) # 1000 <freep>
 810:	a02d                	j	83a <free+0x3c>
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;
  if(bp + bp->s.size == p->s.ptr){
    bp->s.size += p->s.ptr->s.size;
 812:	4618                	lw	a4,8(a2)
 814:	9f2d                	addw	a4,a4,a1
 816:	fee52c23          	sw	a4,-8(a0)
    bp->s.ptr = p->s.ptr->s.ptr;
 81a:	6398                	ld	a4,0(a5)
 81c:	6310                	ld	a2,0(a4)
 81e:	a83d                	j	85c <free+0x5e>
  } else
    bp->s.ptr = p->s.ptr;
  if(p + p->s.size == bp){
    p->s.size += bp->s.size;
 820:	ff852703          	lw	a4,-8(a0)
 824:	9f31                	addw	a4,a4,a2
 826:	c798                	sw	a4,8(a5)
    p->s.ptr = bp->s.ptr;
 828:	ff053683          	ld	a3,-16(a0)
 82c:	a091                	j	870 <free+0x72>
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
 82e:	6398                	ld	a4,0(a5)
 830:	00e7e463          	bltu	a5,a4,838 <free+0x3a>
 834:	00e6ea63          	bltu	a3,a4,848 <free+0x4a>
{
 838:	87ba                	mv	a5,a4
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
 83a:	fed7fae3          	bgeu	a5,a3,82e <free+0x30>
 83e:	6398                	ld	a4,0(a5)
 840:	00e6e463          	bltu	a3,a4,848 <free+0x4a>
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
 844:	fee7eae3          	bltu	a5,a4,838 <free+0x3a>
  if(bp + bp->s.size == p->s.ptr){
 848:	ff852583          	lw	a1,-8(a0)
 84c:	6390                	ld	a2,0(a5)
 84e:	02059813          	slli	a6,a1,0x20
 852:	01c85713          	srli	a4,a6,0x1c
 856:	9736                	add	a4,a4,a3
 858:	fae60de3          	beq	a2,a4,812 <free+0x14>
    bp->s.ptr = p->s.ptr->s.ptr;
 85c:	fec53823          	sd	a2,-16(a0)
  if(p + p->s.size == bp){
 860:	4790                	lw	a2,8(a5)
 862:	02061593          	slli	a1,a2,0x20
 866:	01c5d713          	srli	a4,a1,0x1c
 86a:	973e                	add	a4,a4,a5
 86c:	fae68ae3          	beq	a3,a4,820 <free+0x22>
    p->s.ptr = bp->s.ptr;
 870:	e394                	sd	a3,0(a5)
  } else
    p->s.ptr = bp;
  freep = p;
 872:	00000717          	auipc	a4,0x0
 876:	78f73723          	sd	a5,1934(a4) # 1000 <freep>
}
 87a:	6422                	ld	s0,8(sp)
 87c:	0141                	addi	sp,sp,16
 87e:	8082                	ret

0000000000000880 <malloc>:
  return freep;
}

void*
malloc(uint nbytes)
{
 880:	7139                	addi	sp,sp,-64
 882:	fc06                	sd	ra,56(sp)
 884:	f822                	sd	s0,48(sp)
 886:	f426                	sd	s1,40(sp)
 888:	ec4e                	sd	s3,24(sp)
 88a:	0080                	addi	s0,sp,64
  Header *p, *prevp;
  uint nunits;

  nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
 88c:	02051493          	slli	s1,a0,0x20
 890:	9081                	srli	s1,s1,0x20
 892:	04bd                	addi	s1,s1,15
 894:	8091                	srli	s1,s1,0x4
 896:	0014899b          	addiw	s3,s1,1
 89a:	0485                	addi	s1,s1,1
  if((prevp = freep) == 0){
 89c:	00000517          	auipc	a0,0x0
 8a0:	76453503          	ld	a0,1892(a0) # 1000 <freep>
 8a4:	c915                	beqz	a0,8d8 <malloc+0x58>
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
 8a6:	611c                	ld	a5,0(a0)
    if(p->s.size >= nunits){
 8a8:	4798                	lw	a4,8(a5)
 8aa:	08977a63          	bgeu	a4,s1,93e <malloc+0xbe>
 8ae:	f04a                	sd	s2,32(sp)
 8b0:	e852                	sd	s4,16(sp)
 8b2:	e456                	sd	s5,8(sp)
 8b4:	e05a                	sd	s6,0(sp)
  if(nu < 4096)
 8b6:	8a4e                	mv	s4,s3
 8b8:	0009871b          	sext.w	a4,s3
 8bc:	6685                	lui	a3,0x1
 8be:	00d77363          	bgeu	a4,a3,8c4 <malloc+0x44>
 8c2:	6a05                	lui	s4,0x1
 8c4:	000a0b1b          	sext.w	s6,s4
  p = sbrk(nu * sizeof(Header));
 8c8:	004a1a1b          	slliw	s4,s4,0x4
        p->s.size = nunits;
      }
      freep = prevp;
      return (void*)(p + 1);
    }
    if(p == freep)
 8cc:	00000917          	auipc	s2,0x0
 8d0:	73490913          	addi	s2,s2,1844 # 1000 <freep>
  if(p == SBRK_ERROR)
 8d4:	5afd                	li	s5,-1
 8d6:	a081                	j	916 <malloc+0x96>
 8d8:	f04a                	sd	s2,32(sp)
 8da:	e852                	sd	s4,16(sp)
 8dc:	e456                	sd	s5,8(sp)
 8de:	e05a                	sd	s6,0(sp)
    base.s.ptr = freep = prevp = &base;
 8e0:	00001797          	auipc	a5,0x1
 8e4:	92878793          	addi	a5,a5,-1752 # 1208 <base>
 8e8:	00000717          	auipc	a4,0x0
 8ec:	70f73c23          	sd	a5,1816(a4) # 1000 <freep>
 8f0:	e39c                	sd	a5,0(a5)
    base.s.size = 0;
 8f2:	0007a423          	sw	zero,8(a5)
    if(p->s.size >= nunits){
 8f6:	b7c1                	j	8b6 <malloc+0x36>
        prevp->s.ptr = p->s.ptr;
 8f8:	6398                	ld	a4,0(a5)
 8fa:	e118                	sd	a4,0(a0)
 8fc:	a8a9                	j	956 <malloc+0xd6>
  hp->s.size = nu;
 8fe:	01652423          	sw	s6,8(a0)
  free((void*)(hp + 1));
 902:	0541                	addi	a0,a0,16
 904:	efbff0ef          	jal	7fe <free>
  return freep;
 908:	00093503          	ld	a0,0(s2)
      if((p = morecore(nunits)) == 0)
 90c:	c12d                	beqz	a0,96e <malloc+0xee>
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
 90e:	611c                	ld	a5,0(a0)
    if(p->s.size >= nunits){
 910:	4798                	lw	a4,8(a5)
 912:	02977263          	bgeu	a4,s1,936 <malloc+0xb6>
    if(p == freep)
 916:	00093703          	ld	a4,0(s2)
 91a:	853e                	mv	a0,a5
 91c:	fef719e3          	bne	a4,a5,90e <malloc+0x8e>
  p = sbrk(nu * sizeof(Header));
 920:	8552                	mv	a0,s4
 922:	a43ff0ef          	jal	364 <sbrk>
  if(p == SBRK_ERROR)
 926:	fd551ce3          	bne	a0,s5,8fe <malloc+0x7e>
        return 0;
 92a:	4501                	li	a0,0
 92c:	7902                	ld	s2,32(sp)
 92e:	6a42                	ld	s4,16(sp)
 930:	6aa2                	ld	s5,8(sp)
 932:	6b02                	ld	s6,0(sp)
 934:	a03d                	j	962 <malloc+0xe2>
 936:	7902                	ld	s2,32(sp)
 938:	6a42                	ld	s4,16(sp)
 93a:	6aa2                	ld	s5,8(sp)
 93c:	6b02                	ld	s6,0(sp)
      if(p->s.size == nunits)
 93e:	fae48de3          	beq	s1,a4,8f8 <malloc+0x78>
        p->s.size -= nunits;
 942:	4137073b          	subw	a4,a4,s3
 946:	c798                	sw	a4,8(a5)
        p += p->s.size;
 948:	02071693          	slli	a3,a4,0x20
 94c:	01c6d713          	srli	a4,a3,0x1c
 950:	97ba                	add	a5,a5,a4
        p->s.size = nunits;
 952:	0137a423          	sw	s3,8(a5)
      freep = prevp;
 956:	00000717          	auipc	a4,0x0
 95a:	6aa73523          	sd	a0,1706(a4) # 1000 <freep>
      return (void*)(p + 1);
 95e:	01078513          	addi	a0,a5,16
  }
}
 962:	70e2                	ld	ra,56(sp)
 964:	7442                	ld	s0,48(sp)
 966:	74a2                	ld	s1,40(sp)
 968:	69e2                	ld	s3,24(sp)
 96a:	6121                	addi	sp,sp,64
 96c:	8082                	ret
 96e:	7902                	ld	s2,32(sp)
 970:	6a42                	ld	s4,16(sp)
 972:	6aa2                	ld	s5,8(sp)
 974:	6b02                	ld	s6,0(sp)
 976:	b7f5                	j	962 <malloc+0xe2>
