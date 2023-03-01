all: $(filter-out Makefile %.sh %.py %.png %.csv data,$(wildcard *))

.PHONY: $(filter-out Makefile %.sh %.py %.png %.csv data,$(wildcard *)) clean $(addprefix clean,$(filter-out Makefile %.sh %.py %.png %.csv data,$(wildcard *)))

$(filter-out Makefile %.sh %.py %.png %.csv data,$(wildcard *)):
	$(MAKE) -C $@

clean: $(addprefix clean_,$(filter-out Makefile %.sh %.py %.png %.csv data,$(wildcard *)))
	@echo $+
	

$(addprefix clean_,$(filter-out Makefile %.sh %.py %.png %.csv data,$(wildcard *))):
	@echo clean $@
	$(MAKE) -C $(subst clean_,,$@) clean
