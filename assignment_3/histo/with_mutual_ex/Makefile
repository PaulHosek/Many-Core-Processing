all: $(filter-out Makefile %.sh %.py %.csv,$(wildcard *))

.PHONY: $(filter-out Makefile %.sh %.py %.csv,$(wildcard *)) clean $(addprefix clean,$(filter-out Makefile %.sh %.py %.csv,$(wildcard *)))

$(filter-out Makefile %.sh %.py %.csv,$(wildcard *)):
	$(MAKE) -C $@

clean: $(addprefix clean_,$(filter-out Makefile %.sh %.py %.csv,$(wildcard *)))
	@echo $+
	

$(addprefix clean_,$(filter-out Makefile %.sh %.py %.csv,$(wildcard *))):
	@echo clean $@
	$(MAKE) -C $(subst clean_,,$@) clean
