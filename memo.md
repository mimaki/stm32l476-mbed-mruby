# STM32L476RG メモ

## メモリマップ変更

mbed-os/targes/TARGET_STM/TARGET_STM32L4/TARGET_STM32L476xG/device/TOOLCHAIN_GCC_ARM/STM32L476xx.ld

RAM2 0x10000188 - 0x10007FFF 32KB - 188
RAM1 0x20000000 - 0x20017FFF 96KB

.data, .bss などを RAM2 に配置
RAMをHEAPに割り当て
→ malloc動作せず

em_mallocを自作
ブロックサイズ 8バイト でmrb_openまでは行けた模様
→ 実は途中でこけてNULLが返っている

mrb_load_irep が動作しない？？？

## メモリ削減
- load.cでmrubyバイナリのDBGセクション(デフォルトでは存在しない)とLVARセクション(irep->lv)を読み込まないように修正

# やったらよさそうなこと
## mrb_irepのメンバ削除
- dbg関連
- lvar関連
