#ifndef INVOICE_DATA_H
#define INVOICE_DATA_H

#include <stdint.h>
#include "pc_component_data.h"

#define MAXIMUM_INVOICES 256
#define MAXIMUM_INVOICE_COMPONENTS 32
#define INVOICE_FILE "pc_invoice_csv.csv"

enum assembly_status
{
	STATUS_PENDING = 0,
	STATUS_ASSEMBLING,
	STATUS_COMPLETED,
	STATUS_CANCELLED,
	STATUS_COUNT
};

extern const char *status_names[STATUS_COUNT];

struct invoice
{
	uint32_t id;
	char customer_name[128];
	enum assembly_status status;
	uint32_t component_ids[MAXIMUM_INVOICE_COMPONENTS];
	uint32_t comp_count;
};

struct invoice_manager
{
	struct invoice invoices[MAXIMUM_INVOICES];
	uint32_t count;
	uint32_t next_id;
};

#endif /* INVOICE_DATA_H */