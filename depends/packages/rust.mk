package=rust
$(package)_version=1.42.0
$(package)_download_path=https://depends.pivx.org
$(package)_file_name_linux=rust-$($(package)_version)-x86_64-unknown-linux-gnu.tar.gz
$(package)_sha256_hash_linux=fce33280b344ab0fecc55be24632a3c0086fd763f4df1d59013dfc0e49f0356c
$(package)_file_name_darwin=rust-$($(package)_version)-x86_64-apple-darwin.tar.gz
$(package)_sha256_hash_darwin=db1055c46e0d54b99da05e88c71fea21b3897e74a4f5ff9390e934f3f050c0a8
$(package)_file_name_freebsd=rust-$($(package)_version)-x86_64-unknown-freebsd.tar.gz
$(package)_sha256_hash_freebsd=230bcf17e4383fba85d3c87fe25d17737459fe561a5f4668fe70dcac2da4e17c

# Mapping from GCC canonical hosts to Rust targets
# If a mapping is not present, we assume they are identical, unless $host_os is
# "darwin", in which case we assume x86_64-apple-darwin.
$(package)_rust_target_x86_64-w64-mingw32=x86_64-pc-windows-gnu
$(package)_rust_target_i686-pc-linux-gnu=i686-unknown-linux-gnu
$(package)_rust_target_riscv64-linux-gnu=riscv64gc-unknown-linux-gnu
$(package)_rust_target_riscv64-unknown-linux-gnu=riscv64gc-unknown-linux-gnu
$(package)_rust_target_x86_64-linux-gnu=x86_64-unknown-linux-gnu
$(package)_rust_target_x86_64-pc-linux-gnu=x86_64-unknown-linux-gnu

# Mapping from Rust targets to SHA-256 hashes
$(package)_rust_std_sha256_hash_arm-unknown-linux-gnueabihf=538790218a9515ef8e32c29a3f6d0d242281345fc8a5f0b0e1452bd350633e15
$(package)_rust_std_sha256_hash_aarch64-unknown-linux-gnu=0d32e9600074b0b5e741d154dc72874e598a34e3b33818a0dc9d7a2243d75b7a
$(package)_rust_std_sha256_hash_i686-unknown-linux-gnu=5bc82c00a62f5da9d3b6573a9880b1aa163cc524a02579a04a0889abc3020fc9
$(package)_rust_std_sha256_hash_x86_64-unknown-linux-gnu=face97f81c746b884ac9a6062b2ac99b1f4c314199d0a90c57a0adb0a01dcdc8
$(package)_rust_std_sha256_hash_riscv64gc-unknown-linux-gnu=4029ef6a767018af3a32f7ffa5aeef728dc763fc541e95395cf9924c1b9ca6f0
$(package)_rust_std_sha256_hash_x86_64-apple-darwin=1d61e9ed5d29e1bb4c18e13d551c6d856c73fb8b410053245dc6e0d3b3a0e92c
$(package)_rust_std_sha256_hash_x86_64-pc-windows-gnu=8a8389f3860df6f42fbf8b76a62ddc7b9b6fe6d0fb526dcfc42faab1005bfb6d

define rust_target
$(if $($(1)_rust_target_$(2)),$($(1)_rust_target_$(2)),$(if $(findstring darwin,$(3)),x86_64-apple-darwin,$(2)))
endef

ifneq ($(canonical_host),$(build))
$(package)_rust_target=$(call rust_target,$(package),$(canonical_host),$(host_os))
$(package)_exact_file_name=rust-std-$($(package)_version)-$($(package)_rust_target).tar.gz
$(package)_exact_sha256_hash=$($(package)_rust_std_sha256_hash_$($(package)_rust_target))
$(package)_build_subdir=buildos
$(package)_extra_sources=$($(package)_file_name_$(build_os))

define $(package)_fetch_cmds
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_download_file),$($(package)_file_name),$($(package)_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_file_name_$(build_os)),$($(package)_file_name_$(build_os)),$($(package)_sha256_hash_$(build_os)))
endef

define $(package)_extract_cmds
  mkdir -p $($(package)_extract_dir) && \
  echo "$($(package)_sha256_hash)  $($(package)_source)" > $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_sha256_hash_$(build_os))  $($(package)_source_dir)/$($(package)_file_name_$(build_os))" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  $(build_SHA256SUM) -c $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  mkdir $(canonical_host) && \
  tar --strip-components=1 -xf $($(package)_source) -C $(canonical_host) && \
  mkdir buildos && \
  tar --strip-components=1 -xf $($(package)_source_dir)/$($(package)_file_name_$(build_os)) -C buildos
endef

define $(package)_stage_cmds
  bash ./install.sh --destdir=$($(package)_staging_dir) --prefix=$(host_prefix)/native --disable-ldconfig && \
  ../$(canonical_host)/install.sh --destdir=$($(package)_staging_dir) --prefix=$(host_prefix)/native --disable-ldconfig
endef
else

define $(package)_stage_cmds
  bash ./install.sh --destdir=$($(package)_staging_dir) --prefix=$(host_prefix)/native --disable-ldconfig
endef
endif
