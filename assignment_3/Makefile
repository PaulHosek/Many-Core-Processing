all: $(filter-out Makefile README.md Experiment_setup,$(wildcard *))

.PHONY: $(filter-out Makefile README.md Experiment_setup,$(wildcard *)) clean $(addprefix clean,$(filter-out Makefile README.md Experiment_setup,$(wildcard *)))

$(filter-out Makefile README.md Experiment_setup,$(wildcard *)):
	$(MAKE) -C $@

clean: $(addprefix clean_,$(filter-out Makefile README.md Experiment_setup,$(wildcard *)))
	@echo $+
	

$(addprefix clean_,$(filter-out Makefile README.md Experiment_setup,$(wildcard *))):
	@echo clean $@
	$(MAKE) -C $(subst clean_,,$@) clean
