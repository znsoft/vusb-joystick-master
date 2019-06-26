/**
 * @brief   Initialize ports and timers
 * @param   void
 * @return  none
 */
extern void encode_init(void);

/**
 * @brief   Read single step encoders
 * @param   void
 * @return  counts since last call
 */
int8_t encode_read1(void);

/**
 * @brief   Read two step encoders
 * @param   void
 * @return  counts since last call
 */
int8_t encode_read2(void);

/**
 * @brief   Read four step encoders
 * @param   void
 * @return  counts since last call
 */
int8_t encode_read4(void);
