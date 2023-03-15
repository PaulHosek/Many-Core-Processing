all: $(filter-out Makefile %.sh %.py,$(wildcard *))

.PHONY: $(filter-out Makefile %.sh %.py,$(wildcard *)) clean $(addprefix clean,$(filter-out Makefile %.sh %.py,$(wildcard *)))

$(filter-out Makefile %.sh %.py,$(wildcard *)):
	$(MAKE) -C $@

clean: $(addprefix clean_,$(filter-out Makefile %.sh %.py,$(wildcard *)))
	@echo $+
	

$(addprefix clean_,$(filter-out Makefile %.sh %.py,$(wildcard *))):
	@echo clean $@
	$(MAKE) -C $(subst clean_,,$@) clean
