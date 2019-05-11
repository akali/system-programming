#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>



int start(void) {
  
	char * envp[] = { "HOME=/", NULL };
	char * argv[] = { "/bin/bash", "-c /bin/ls >> /home/aigerim/list", NULL};

	int rc = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);

	printk(KERN_INFO "rc=%d\n", rc);

}

void end(void) {
    pr_info("Ending ");
}


module_init(start);
module_exit(end);