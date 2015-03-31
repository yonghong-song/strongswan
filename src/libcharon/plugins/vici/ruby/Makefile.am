EXTRA_DIST = vici.gemspec.in lib/vici.rb

vici.gemspec: $(srcdir)/vici.gemspec.in
	$(AM_V_GEN) sed \
	-e "s:@GEM_VERSION@:$(PACKAGE_VERSION):" \
	$(srcdir)/vici.gemspec.in > $@

vici-$(PACKAGE_VERSION).gem: vici.gemspec $(EXTRA_DIST)
	(cd $(srcdir); $(GEM) build $(abs_builddir)/vici.gemspec)
	[ "$(srcdir)" = "$(builddir)" ] || \
		mv $(srcdir)/vici-$(PACKAGE_VERSION).gem $(builddir)

all-local: vici-$(PACKAGE_VERSION).gem

clean-local:
	rm -f vici.gemspec vici-$(PACKAGE_VERSION).gem

rubygem_DATA = vici-$(PACKAGE_VERSION).gem
