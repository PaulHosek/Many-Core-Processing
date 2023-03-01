all: $(filter-out Makefile README.md test,$(wildcard *))

.PHONY: $(filter-out Makefile README.md test,$(wildcard *)) clean $(addprefix clean,$(filter-out Makefile README.md test,$(wildcard *)))

$(filter-out Makefile README.md test,$(wildcard *)):
	$(MAKE) -C $@

clean: $(addprefix clean_,$(filter-out Makefile README.md test,$(wildcard *)))
	@echo $+
	

$(addprefix clean_,$(filter-out Makefile README.md test,$(wildcard *))):
	@echo clean $@
	$(MAKE) -C $(subst clean_,,$@) clean
