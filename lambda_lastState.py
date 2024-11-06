import json
import boto3

def lambda_handler(event, context):
    # Inizializza il client S3 e definisci il bucket e il percorso del file
    s3_client = boto3.client('s3')
    bucket_name = 'agvbucket'  # Sostituisci con il nome del tuo bucket S3
    s3_key = 'logs.json'
    
    try:
        # Ottieni il contenuto di logs.json da S3
        s3_object = s3_client.get_object(Bucket=bucket_name, Key=s3_key)
        logs_data = json.loads(s3_object['Body'].read())
        
        # Controlla che ci siano log nel file
        if "logs" in logs_data and len(logs_data["logs"]) > 0:
            # Recupera l'ultimo log
            last_log = logs_data["logs"][-1]
            
            # Restituisce l'ultimo log come risposta JSON
            return {
                'statusCode': 200,
                'body': json.dumps({
                    'last_log': last_log
                })
            }
        else:
            # Risposta se non ci sono log nel file
            return {
                'statusCode': 200,
                'body': json.dumps({
                    'message': 'Nessun log trovato.'
                })
            }
            
    except Exception as e:
        # Gestione degli errori
        return {
            'statusCode': 500,
            'body': json.dumps({
                'error': str(e),
                'message': 'Errore nel recupero del file logs.json da S3.'
            })
        }
